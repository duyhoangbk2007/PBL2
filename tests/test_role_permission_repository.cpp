#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../src/data_access/RolePermissionRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << endl;
    (condition ? passed : failed)++;
}

struct TestFiles
{
    string valid;
    string invalid;
    string missing;

    TestFiles()
    {
        const string suffix = to_string(chrono::steady_clock::now()
                                            .time_since_epoch().count());
        const filesystem::path dir = filesystem::temp_directory_path();
        valid = (dir / ("pbl2_role_permissions_valid_" + suffix + ".txt")).string();
        invalid = (dir / ("pbl2_role_permissions_invalid_" + suffix + ".txt")).string();
        missing = (dir / ("pbl2_role_permissions_missing_" + suffix + ".txt")).string();
    }

    ~TestFiles()
    {
        error_code ignored;
        filesystem::remove(valid, ignored);
        filesystem::remove(invalid, ignored);
        filesystem::remove(missing, ignored);
    }
};

void writeTestFile(const string& path, const string& data)
{
    ofstream file(path, ios::binary);
    if (!file || !(file << data))
    {
        throw runtime_error("Khong ghi duoc file test: " + path);
    }
}

const vector<string> ALL_PERMISSIONS = {
    "MOVIE_VIEW", "MOVIE_ADD", "MOVIE_UPDATE", "MOVIE_DELETE",
    "ROOM_VIEW", "ROOM_ADD", "ROOM_UPDATE", "ROOM_DELETE",
    "SEAT_VIEW", "SEAT_ADD", "SEAT_UPDATE", "SEAT_DELETE",
    "SHOWTIME_VIEW", "SHOWTIME_ADD", "SHOWTIME_UPDATE", "SHOWTIME_CANCEL",
    "SHOWTIME_DELETE", "STAFF_VIEW", "STAFF_ADD", "STAFF_UPDATE",
    "STAFF_DEACTIVATE", "MEMBER_VIEW", "MEMBER_ADD", "MEMBER_UPDATE",
    "TICKET_SELL", "TICKET_CANCEL", "INVOICE_VIEW_OWN",
    "INVOICE_VIEW_ALL", "REPORT_VIEW", "PRICING_VIEW", "PRICING_UPDATE"
};

void testInitialAndValidLoad(const TestFiles& files)
{
    cout << "\n===== TEST INITIAL / VALID LOAD =====" << endl;
    RolePermissionRepository repo(files.valid);
    check(!repo.isLoaded() &&
          !repo.hasPermission("ADMIN", "MOVIE_ADD") &&
          !repo.hasPermission("STAFF", "TICKET_SELL"),
          "Trang thai ban dau va tra quyen truoc load");

    string data;
    for (const string& permission : ALL_PERMISSIONS)
    {
        data += "ADMIN|" + permission + "\n";
    }
    data += "STAFF|MOVIE_VIEW\n";
    data += "STAFF|TICKET_SELL\n";
    writeTestFile(files.valid, data);
    check(repo.load() && repo.isLoaded(), "Load file hop le");
    check(ALL_PERMISSIONS.size() == 31, "Danh muc test co 31 ma quyen");
    for (const string& permission : ALL_PERMISSIONS)
    {
        check(repo.hasPermission("ADMIN", permission),
              "Load va tra ma quyen: " + permission);
    }
    check(repo.hasPermission("ADMIN", "MOVIE_VIEW") &&
          repo.hasPermission("STAFF", "MOVIE_VIEW"),
          "Hai role cung mot permission");
    check(repo.hasPermission("STAFF", "TICKET_SELL") &&
          !repo.hasPermission("STAFF", "MOVIE_ADD"),
          "STAFF chi co quyen trong TXT");
    check(!repo.hasPermission("ADMIN", "UNKNOWN_PERMISSION") &&
          !repo.hasPermission("GUEST", "MOVIE_VIEW") &&
          !repo.hasPermission("ADMIN", "movie_view") &&
          !repo.hasPermission("admin", "MOVIE_VIEW"),
          "Role, permission khong hop le va phan biet chu hoa");

    writeTestFile(files.valid, "STAFF|MOVIE_VIEW\n");
    check(repo.load() && repo.isLoaded() &&
          repo.hasPermission("STAFF", "MOVIE_VIEW") &&
          !repo.hasPermission("ADMIN", "MOVIE_VIEW") &&
          !repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "ADMIN khong tu cap quyen; nap lai thu hoi quyen cu");
    writeTestFile(files.valid, "");
    check(repo.load() && repo.isLoaded() &&
          !repo.hasPermission("STAFF", "MOVIE_VIEW"),
          "Load file rong khong cap quyen");
}

void testInvalidLoad(const TestFiles& files)
{
    cout << "\n===== TEST INVALID LOAD =====" << endl;
    struct InvalidCase
    {
        string name;
        string data;
        int line;
        string reason;
    };
    const vector<InvalidCase> cases = {
        {"thieu dau phan cach", "ADMIN_MOVIE_ADD\n", 1, "Sai dinh dang"},
        {"thua dau phan cach", "ADMIN|MOVIE|ADD\n", 1, "Sai dinh dang"},
        {"role rong", "|MOVIE_ADD\n", 1, "Sai dinh dang"},
        {"permission rong", "ADMIN|\n", 1, "Sai dinh dang"},
        {"role khong hop le", "GUEST|MOVIE_ADD\n", 1, "Role khong hop le"},
        {"permission ngoai danh muc", "ADMIN|UNKNOWN_PERMISSION\n", 1,
         "Permission khong hop le"},
        {"role co khoang trang", "ADMIN |MOVIE_ADD\n", 1, "Role khong hop le"},
        {"permission co khoang trang", "ADMIN| MOVIE_ADD\n", 1,
         "Permission khong hop le"},
        {"trung cap role va permission", "ADMIN|MOVIE_ADD\nADMIN|MOVIE_ADD\n", 2,
         "Trung cap role va permission"},
        {"dong hop le truoc dong sai", "ADMIN|MOVIE_ADD\nSTAFF|BAD_CODE\n", 2,
         "Permission khong hop le"}
    };
    RolePermissionRepository repo(files.invalid);
    for (const InvalidCase& item : cases)
    {
        writeTestFile(files.invalid, item.data);
        bool threw = false;
        string message;
        try
        {
            repo.load();
        }
        catch (const runtime_error& error)
        {
            threw = true;
            message = error.what();
        }
        check(threw && message.find(files.invalid) != string::npos &&
              message.find("tai dong " + to_string(item.line)) != string::npos &&
              message.find(item.reason) != string::npos,
              "Runtime_error co file, dong, nguyen nhan: " + item.name);
        check(!repo.isLoaded() &&
              !repo.hasPermission("ADMIN", "MOVIE_ADD") &&
              !repo.hasPermission("STAFF", "MOVIE_VIEW"),
              "Load loi khong cap quyen: " + item.name);
    }
}

void testReloadAndMissing(const TestFiles& files)
{
    cout << "\n===== TEST RELOAD / MISSING =====" << endl;
    writeTestFile(files.valid, "ADMIN|MOVIE_VIEW\n");
    RolePermissionRepository repo(files.valid);
    check(repo.load() && repo.hasPermission("ADMIN", "MOVIE_VIEW"),
          "Load ban dau cho nap lai");

    writeTestFile(files.valid, "ADMIN|MOVIE_VIEW\nSTAFF|TICKET_SELL\n");
    check(repo.load() && repo.hasPermission("STAFF", "TICKET_SELL"),
          "Nap lai cap them quyen tu TXT");
    writeTestFile(files.valid, "STAFF|TICKET_SELL\n");
    check(repo.load() && !repo.hasPermission("ADMIN", "MOVIE_VIEW") &&
          repo.hasPermission("STAFF", "TICKET_SELL"),
          "Nap lai thu hoi quyen tu TXT");

    writeTestFile(files.valid, "");
    check(repo.load() && repo.isLoaded() &&
          !repo.hasPermission("STAFF", "TICKET_SELL"),
          "Nap lai file rong");
    writeTestFile(files.valid, "ADMIN|MOVIE_ADD\n");
    check(repo.load() && repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Tao quyen cu truoc nap lai loi");
    writeTestFile(files.valid, "ADMIN|MOVIE_ADD\nSTAFF|UNKNOWN\n");
    bool threw = false;
    string message;
    try
    {
        repo.load();
    }
    catch (const runtime_error& error)
    {
        threw = true;
        message = error.what();
    }
    check(threw && message.find(files.valid) != string::npos &&
          message.find("tai dong 2") != string::npos &&
          !repo.isLoaded() && !repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Nap lai file loi thu hoi toan bo quyen cu");
    writeTestFile(files.valid, "STAFF|ROOM_VIEW\n");
    check(repo.load() && repo.isLoaded() &&
          repo.hasPermission("STAFF", "ROOM_VIEW") &&
          !repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Sua file va nap lai thanh cong sau loi");

    error_code ignored;
    filesystem::remove(files.missing, ignored);
    RolePermissionRepository missing(files.missing);
    check(!missing.load() && !missing.isLoaded() &&
          !missing.hasPermission("ADMIN", "MOVIE_VIEW"),
          "File khong ton tai tra false");
    filesystem::remove(files.valid, ignored);
    check(!repo.load() && !repo.isLoaded() &&
          !repo.hasPermission("STAFF", "ROOM_VIEW"),
          "Nap lai file khong ton tai thu hoi quyen cu");
}

int main()
{
    try
    {
        TestFiles files;
        testInitialAndValidLoad(files);
        testInvalidLoad(files);
        testReloadAndMissing(files);
    }
    catch (const exception& error)
    {
        failed++;
        cout << "[ERROR] " << error.what() << endl;
    }
    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
