#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "../src/business/StaffService.h"
#include "../src/data_access/UserRepository.h"
#include "../src/models/Invoice.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << endl;
    (condition ? passed : failed)++;
}

template <typename F>
bool throwsError(F action)
{
    try { action(); }
    catch (const exception&) { return true; }
    return false;
}

void writeFile(const fs::path& path, const string& data)
{
    ofstream out(path, ios::binary);
    if (!out || !(out << data)) throw runtime_error("Khong ghi duoc TXT test");
}

string readFile(const fs::path& path)
{
    ifstream in(path, ios::binary);
    if (!in) throw runtime_error("Khong doc duoc TXT test");
    string data;
    for (char ch; in.get(ch);)
    {
        if (ch != '\r') data += ch;
    }
    return data;
}

struct TestFiles
{
    fs::path dir;
    TestFiles()
    {
        static int sequence = 0;
        dir = fs::temp_directory_path() /
              ("pbl2_staff_service_" +
               to_string(chrono::steady_clock::now().time_since_epoch().count()) +
               "_" + to_string(++sequence));
        fs::create_directories(dir);
    }
    ~TestFiles()
    {
        error_code error;
        fs::remove_all(dir, error);
    }
    fs::path path(const string& name) const { return dir / name; }
};

const string USERS =
    "USR0001|admin|secret|Admin|ADMIN|1\n"
    "USR0002|staff|secret|Staff|STAFF|1\n"
    "USR0003|off|secret|Inactive|STAFF|0\n";
const string RIGHTS =
    "ADMIN|STAFF_VIEW\nADMIN|STAFF_ADD\nADMIN|STAFF_UPDATE\n"
    "ADMIN|STAFF_DEACTIVATE\nSTAFF|STAFF_VIEW\nSTAFF|STAFF_ADD\n"
    "STAFF|STAFF_UPDATE\nSTAFF|STAFF_DEACTIVATE\n";

struct Fixture
{
    TestFiles files;
    UserRepository concrete;
    IRepository<User>& users;
    RolePermissionRepository permissions;
    AuthService auth;
    StaffService service;

    Fixture(const string& data = USERS, const string& rights = RIGHTS)
        : concrete(files.path("users.txt").string()), users(concrete),
          permissions(files.path("permissions.txt").string()),
          auth(users, permissions), service(users, auth)
    {
        writeFile(files.path("users.txt"), data);
        writeFile(files.path("permissions.txt"), rights);
        if (!permissions.load()) throw runtime_error("Khong nap duoc quyen test");
    }
    void login(const string& name = "admin")
    {
        if (!auth.login(name, "secret")) throw runtime_error("Login test that bai");
    }
};

struct MockUsers : IRepository<User>
{
    User admin{"USR0001", "admin", "secret", "Admin", UserRole::ADMIN, true};
    User staff{"USR0002", "staff", "secret", "Staff", UserRole::STAFF, true};
    bool loaded = true;
    bool addThrows = false;
    bool updateThrows = false;
    bool nullStaff = false;
    int updateCalls = 0;
    bool load() override { return true; }
    bool save() override { return true; }
    bool add(User*) override
    {
        if (addThrows) throw runtime_error("Mock add failed");
        return false;
    }
    bool update(const User&) override
    {
        updateCalls++;
        if (updateThrows) throw runtime_error("Mock update failed");
        return false;
    }
    bool removeById(const string&) override { return false; }
    User* findById(const string& id) override
    {
        if (id == admin.getUserId()) return &admin;
        if (id == staff.getUserId()) return &staff;
        return nullptr;
    }
    User* getAt(int index) override
    {
        if (index == 0) return &admin;
        if (index == 1) return nullStaff ? nullptr : &staff;
        return nullptr;
    }
    int count() override { return 2; }
    bool isLoaded() override { return loaded; }
};

void testLoadSaveAndDIP()
{
    cout << "\n===== TEST LOAD / SAVE / DIP =====" << endl;
    check(is_base_of<IRepository<User>, UserRepository>::value &&
          is_constructible<StaffService, IRepository<User>&, AuthService&>::value,
          "Constructor DI qua IRepository<User>&");
    Fixture f;
    check(!f.service.save() && f.service.load() && !f.service.load(),
          "Save truoc load, load hop le va load lap");
    f.login();
    check(f.service.save() && readFile(f.files.path("users.txt")) == USERS,
          "Save dung schema va khong doi du lieu");
    Fixture reload(readFile(f.files.path("users.txt")));
    check(reload.service.load() && reload.users.count() == 3,
          "Save roi load bang fixture rieng");
    Fixture missing;
    fs::remove(missing.files.path("users.txt"));
    check(!missing.service.load() && !missing.users.isLoaded(),
          "Load file thieu");
    Fixture malformed("USR0001|bad\n");
    check(throwsError([&] { malformed.service.load(); }) &&
          !malformed.users.isLoaded(), "Load TXT sai truyen exception");
}

void testAddAndUpdate()
{
    cout << "\n===== TEST ADD / UPDATE =====" << endl;
    Fixture f;
    f.service.load();
    f.login();
    User input("ORIGINAL", "newstaff", "pw", "New Staff", UserRole::STAFF, true);
    check(!f.service.addStaff(nullptr), "Add nullptr");
    check(f.service.addStaff(&input) && input.getUserId() == "ORIGINAL" &&
          f.users.findById("USR0004") != nullptr &&
          f.users.findById("USR0004") != &input,
          "Sinh ID tu ADMIN va STAFF, caller giu input");
    int count = f.users.count();
    User duplicate("ORIGINAL", "admin", "pw", "Dup", UserRole::STAFF, true);
    User invalid("ORIGINAL", "", "pw", "Bad", UserRole::STAFF, true);
    User wrongRole("ORIGINAL", "other", "pw", "Admin", UserRole::ADMIN, true);
    check(!f.service.addStaff(&duplicate) && !f.service.addStaff(&invalid) &&
          !f.service.addStaff(&wrongRole) && f.users.count() == count &&
          duplicate.getUserId() == "ORIGINAL",
          "Add trung username ADMIN, du lieu sai, role ADMIN bi tu choi");
    User* current = f.users.findById("USR0004");
    User changed("USR0004", "renamed", "newpw", "Renamed", UserRole::STAFF, true);
    check(f.service.updateStaff(changed) && f.users.findById("USR0004") == current &&
          current->getUsername() == "renamed" && current->getPassword() == "newpw" &&
          current->getUserId() == "USR0004", "Update hop le giu ID, dia chi");
    User missing("USR9999", "missing", "pw", "Missing", UserRole::STAFF, true);
    User adminTarget("USR0001", "admin", "secret", "Admin", UserRole::ADMIN, true);
    User changedRole("USR0004", "renamed", "pw", "Bad", UserRole::ADMIN, true);
    User changedActive("USR0004", "renamed", "pw", "Bad", UserRole::STAFF, false);
    User sameName("USR0004", "staff", "pw", "Bad", UserRole::STAFF, true);
    check(!f.service.updateStaff(missing) && !f.service.updateStaff(adminTarget) &&
          !f.service.updateStaff(changedRole) && !f.service.updateStaff(changedActive) &&
          !f.service.updateStaff(sameName) && current->getUsername() == "renamed" &&
          current->isActive(), "Update sai ID, ADMIN, role, active, username trung");
    Fixture exhausted("USR9999|admin|secret|Admin|ADMIN|1\n");
    exhausted.service.load();
    exhausted.login();
    check(throwsError([&] { exhausted.service.addStaff(&input); }) &&
          exhausted.users.count() == 1 && input.getUserId() == "ORIGINAL",
          "Het day ID truyen exception, khong doi input");
}

void testActiveAndQueries()
{
    cout << "\n===== TEST ACTIVE / QUERY =====" << endl;
    Fixture f;
    f.service.load();
    f.login();
    User* staff = f.users.findById("USR0002");
    Invoice invoice("INV0001", "SHW0001", "USR0002", "",
                    "2026-11-27T10:00:00", 100.0);
    check(invoice.linkSeller(staff) && staff->getInvoiceCount() == 1,
          "Lien ket Invoice voi STAFF");
    check(f.service.setStaffActive("USR0002", false) && !staff->isActive() &&
          staff->getUsername() == "staff" && staff->getInvoiceCount() == 1 &&
          staff->getInvoiceAt(0) == &invoice,
          "Vo hieu hoa giu thong tin va lien ket Invoice");
    check(f.service.setStaffActive("USR0002", false) &&
          f.service.setStaffActive("USR0002", true) && staff->isActive() &&
          staff->getInvoiceAt(0) == &invoice,
          "Trang thai da dung va kich hoat lai");
    check(!f.service.setStaffActive("USR9999", false) &&
          !f.service.setStaffActive("USR0001", false) &&
          f.users.findById("USR0001")->isActive(),
          "ID thieu va ADMIN khong duoc doi active");
    check(f.service.findStaffById("USR0002") == staff &&
          f.service.findStaffByUsername("staff") == staff &&
          f.service.findStaffByUsername("Staff") == nullptr &&
          f.service.findStaffById("USR0001") == nullptr &&
          f.service.findStaffByUsername("admin") == nullptr,
          "Find STAFF theo ID/username, phan biet hoa thuong, loc ADMIN");
    check(f.service.getStaffCount() == 2 &&
          f.service.getStaffAt(0) == staff &&
          f.service.getStaffAt(1) == f.users.findById("USR0003") &&
          f.service.getStaffAt(-1) == nullptr &&
          f.service.getStaffAt(2) == nullptr,
          "Count/getAt chi STAFF, gom inactive, index bien");
    invoice.unlinkReferences();

    Fixture adminOnly("USR0001|admin|secret|Admin|ADMIN|1\n");
    adminOnly.service.load();
    adminOnly.login();
    check(adminOnly.service.getStaffCount() == 0 &&
          adminOnly.service.getStaffAt(0) == nullptr,
          "Danh sach khong co STAFF");
}

void testPermissionAndMock()
{
    cout << "\n===== TEST PERMISSION / MOCK =====" << endl;
    Fixture f(USERS, "STAFF|STAFF_VIEW\nSTAFF|STAFF_ADD\n"
                     "STAFF|STAFF_UPDATE\nSTAFF|STAFF_DEACTIVATE\n");
    f.service.load();
    User input("ORIGINAL", "newstaff", "pw", "New Staff", UserRole::STAFF, true);
    check(throwsError([&] { f.service.addStaff(&input); }) &&
          throwsError([&] { f.service.getStaffCount(); }),
          "Chua login bi tu choi");
    f.login();
    check(throwsError([&] { f.service.addStaff(&input); }) &&
          throwsError([&] { f.service.getStaffCount(); }),
          "ADMIN thieu quyen bi tu choi");
    f.login("staff");
    check(f.service.addStaff(&input) && f.service.getStaffCount() == 3,
          "STAFF duoc cap quyen thao tac");
    f.users.findById("USR0002")->setActive(false);
    check(throwsError([&] { f.service.getStaffCount(); }),
          "Tai khoan thao tac inactive bi tu choi");

    Fixture noRights(USERS, "");
    noRights.service.load();
    noRights.login();
    User update("USR0002", "staff", "secret", "Staff", UserRole::STAFF, true);
    check(throwsError([&] { noRights.service.updateStaff(update); }) &&
          throwsError([&] { noRights.service.setStaffActive("USR0002", false); }) &&
          throwsError([&] { noRights.service.findStaffById("USR0002"); }) &&
          throwsError([&] { noRights.service.findStaffByUsername("staff"); }) &&
          throwsError([&] { noRights.service.getStaffAt(0); }),
          "Thieu UPDATE/DEACTIVATE/VIEW bi tu choi");

    TestFiles files;
    writeFile(files.path("permissions.txt"), RIGHTS);
    RolePermissionRepository permissions(files.path("permissions.txt").string());
    permissions.load();
    MockUsers mock;
    IRepository<User>& repo = mock;
    AuthService auth(repo, permissions);
    StaffService service(repo, auth);
    check(auth.login("admin", "secret"), "Mock DI dung chung Repository voi Auth");
    check(!service.addStaff(&input) && input.getUserId() == "ORIGINAL",
          "Mock add false: caller giu input");
    mock.addThrows = true;
    check(throwsError([&] { service.addStaff(&input); }) &&
          input.getUserId() == "ORIGINAL", "Mock add exception truyen lai");
    check(!service.setStaffActive("USR0002", false) &&
          mock.staff.isActive() && mock.updateCalls == 1,
          "Mock update false: khong doi active");
    check(service.setStaffActive("USR0002", true) && mock.updateCalls == 1,
          "Trang thai da dung khong goi update");
    mock.updateThrows = true;
    check(throwsError([&] { service.setStaffActive("USR0002", false); }) &&
          mock.staff.isActive(), "Mock update exception truyen lai");
    mock.nullStaff = true;
    check(service.findStaffByUsername("staff") == nullptr &&
          service.getStaffCount() == 0 && service.getStaffAt(0) == nullptr,
          "Mock getAt nullptr khong dereference");

    MockUsers unloaded;
    unloaded.loaded = false;
    StaffService unloadedService(unloaded, auth);
    User values("USR0002", "staff", "secret", "Staff", UserRole::STAFF, true);
    check(!unloadedService.addStaff(&input) &&
          !unloadedService.updateStaff(values) &&
          !unloadedService.setStaffActive("USR0002", false),
          "Staff Repository chua load: ba thao tac ghi tra false sau Permission");
}

int main()
{
    try
    {
        testLoadSaveAndDIP();
        testAddAndUpdate();
        testActiveAndQueries();
        testPermissionAndMock();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
