
#include "../src/data_access/RolePermissionRepository.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    if (condition)
    {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name << endl;
        failed++;
    }
}

bool writeTestFile(
    const string& path,
    const string& content
)
{
    ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    file << content;
    file.close();
    return !file.fail();
}

int main()
{
    const string path = "test_role_permission_temp.txt";

    // TEST 1: Trang thai ban dau
    RolePermissionRepository repo(path);

    check(!repo.isLoaded(), "Initially not loaded");
    check(!repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Cannot check permission before load");

    // TEST 2: Load file hop le
    bool created = writeTestFile(
        path,
        "ADMIN|MOVIE_ADD\n"
        "ADMIN|MOVIE_DELETE\n"
        "STAFF|MOVIE_VIEW\n"
        "STAFF|ROOM_VIEW\n"
    );

    if (!created)
    {
        cout << "Cannot create test file!" << endl;
        return 1;
    }

    check(repo.load(), "Load valid file");
    check(repo.isLoaded(), "Loaded state is true");

    // TEST 3: Kiem tra quyen ton tai
    check(repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Admin has MOVIE_ADD");

    check(repo.hasPermission("ADMIN", "MOVIE_DELETE"),
          "Admin has MOVIE_DELETE");

    check(repo.hasPermission("STAFF", "MOVIE_VIEW"),
          "Staff has MOVIE_VIEW");

    check(repo.hasPermission("STAFF", "ROOM_VIEW"),
          "Staff has ROOM_VIEW");

    // TEST 4: Kiem tra quyen khong ton tai
    check(!repo.hasPermission("STAFF", "MOVIE_ADD"),
          "Staff does not have MOVIE_ADD");

    check(!repo.hasPermission("ADMIN", "ROOM_VIEW"),
          "Admin does not have unassigned ROOM_VIEW");

    check(!repo.hasPermission("GUEST", "MOVIE_VIEW"),
          "Unknown role has no permission");

    check(!repo.hasPermission("STAFF", "UNKNOWN"),
          "Unknown permission is denied");

    // TEST 5: Load lai file hop le
    writeTestFile(
        path,
        "STAFF|MOVIE_ADD\n"
    );

    check(repo.load(), "Reload updated file");

    check(repo.hasPermission("STAFF", "MOVIE_ADD"),
          "New permission is available");

    check(!repo.hasPermission("STAFF", "MOVIE_VIEW"),
          "Old permission is removed");

    // TEST 6: File khong ton tai
    remove(path.c_str());

    check(!repo.load(), "Load missing file fails");

    check(!repo.isLoaded(),
          "Missing file resets loaded state");

    check(!repo.hasPermission("STAFF", "MOVIE_ADD"),
          "Missing file denies permissions");

    // TEST 7: Sai dinh dang - khong co dau |
    writeTestFile(path, "ADMIN_MOVIE_ADD\n");

    check(!repo.load(), "Reject missing separator");

    // TEST 8: Sai dinh dang - nhieu dau |
    writeTestFile(path, "ADMIN|MOVIE|ADD\n");

    check(!repo.load(), "Reject multiple separators");

    // TEST 9: Role rong
    writeTestFile(path, "|MOVIE_ADD\n");

    check(!repo.load(), "Reject empty role");

    // TEST 10: Permission rong
    writeTestFile(path, "ADMIN|\n");

    check(!repo.load(), "Reject empty permission");

    // TEST 11: Role khong hop le
    writeTestFile(path, "GUEST|MOVIE_VIEW\n");

    check(!repo.load(), "Reject invalid role");

    // TEST 12: Du lieu trung
    writeTestFile(
        path,
        "ADMIN|MOVIE_ADD\n"
        "ADMIN|MOVIE_ADD\n"
    );

    check(!repo.load(), "Reject duplicate permission");

    // TEST 13: Loi o giua file
    writeTestFile(
        path,
        "ADMIN|MOVIE_ADD\n"
        "INVALID_LINE\n"
    );

    check(!repo.load(), "Reject partially invalid file");

    check(!repo.isLoaded(),
          "Invalid file keeps loaded false");

    check(!repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Partial data cannot grant permission");

    // TEST 14: File rong
    writeTestFile(path, "");

    check(repo.load(), "Load empty file");

    check(repo.isLoaded(), "Empty file is loaded");

    check(!repo.hasPermission("ADMIN", "MOVIE_ADD"),
          "Empty file grants no permission");

    // Don dep
    remove(path.c_str());

    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
