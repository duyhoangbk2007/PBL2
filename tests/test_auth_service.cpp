#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "../src/business/AuthService.h"
#include "../src/data_access/IRepository.h"
#include "../src/data_access/UserRepository.h"

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
    try
    {
        action();
    }
    catch (const exception&)
    {
        return true;
    }
    return false;
}

void writeFile(const fs::path& path, const string& data)
{
    ofstream out(path, ios::binary);
    if (!out || !(out << data))
    {
        throw runtime_error("Khong ghi duoc file test: " + path.string());
    }
}

struct TestFiles
{
    fs::path dir;

    TestFiles()
    {
        static int sequence = 0;
        dir = fs::temp_directory_path() /
              ("pbl2_auth_service_" +
               to_string(chrono::steady_clock::now().time_since_epoch().count()) +
               "_" + to_string(++sequence));
        fs::create_directories(dir);
    }

    ~TestFiles()
    {
        error_code error;
        fs::remove_all(dir, error);
    }

    fs::path path(const string& name) const
    {
        return dir / name;
    }
};

const string USERS =
    "USR0001|admin|secret|Admin User|ADMIN|1\n"
    "USR0002|staff|secret|Staff User|STAFF|1\n"
    "USR0003|inactive|secret|Inactive User|STAFF|0\n";

struct Fixture
{
    TestFiles files;
    UserRepository concrete;
    IRepository<User>& users;
    RolePermissionRepository permissions;
    AuthService auth;

    Fixture(const string& rights = "")
        : concrete(files.path("users.txt").string()), users(concrete),
          permissions(files.path("permissions.txt").string()),
          auth(users, permissions)
    {
        writeFile(files.path("users.txt"), USERS);
        writeFile(files.path("permissions.txt"), rights);
    }
};

struct MockUsers : IRepository<User>
{
    User admin{"USR0001", "admin", "secret", "Admin", UserRole::ADMIN, true};
    User staff{"USR0002", "staff", "secret", "Staff", UserRole::STAFF, true};
    User invalidAdmin{"USR0001", "admin", "secret", "Admin",
                      static_cast<UserRole>(99), true};
    bool loaded = true;
    bool missing = false;
    bool nullFirst = false;
    bool invalidRole = false;
    int getAtCalls = 0;
    int findByIdCalls = 0;

    bool load() override { loaded = true; return true; }
    bool save() override { return true; }
    bool add(User*) override { return false; }
    bool update(const User&) override { return false; }
    bool removeById(const string&) override { return false; }
    User* findById(const string& id) override
    {
        findByIdCalls++;
        if (missing) return nullptr;
        if (id == admin.getUserId()) return &admin;
        if (id == staff.getUserId()) return &staff;
        return nullptr;
    }
    User* getAt(int index) override
    {
        getAtCalls++;
        if (index == 0) return nullFirst ? nullptr :
                               (invalidRole ? &invalidAdmin : &admin);
        if (index == 1) return &staff;
        return nullptr;
    }
    int count() override { return 2; }
    bool isLoaded() override { return loaded; }
};

void testConstructorAndLogin()
{
    cout << "\n===== TEST CONSTRUCTOR / LOGIN =====" << endl;
    check(is_base_of<IRepository<User>, UserRepository>::value &&
          is_constructible<AuthService, IRepository<User>&,
                           RolePermissionRepository&>::value,
          "Constructor DI qua IRepository<User>&");
    Fixture f;
    check(f.auth.getCurrentUser() == nullptr && !f.auth.isLoggedIn() &&
          !f.auth.hasPermission("MOVIE_VIEW"), "Phien ban dau rong");
    check(throwsError([&] { f.auth.login("admin", "secret"); }) &&
          f.auth.getCurrentUser() == nullptr,
          "Login khi UserRepo chua load nem exception");
    check(f.users.load(), "Nap UserRepo test");
    check(!f.auth.login("", "secret") &&
          !f.auth.login("admin", "") &&
          !f.auth.login("unknown", "secret") &&
          !f.auth.login("admin", "wrong") &&
          !f.auth.login("inactive", "secret") &&
          !f.auth.isLoggedIn(),
          "Username/password rong, sai, thieu, inactive bi tu choi");
    check(f.auth.login("admin", "secret") && f.auth.isLoggedIn() &&
          f.auth.getCurrentUser() == f.users.findById("USR0001") &&
          &f.auth.requireLogin() == f.users.findById("USR0001"),
          "ADMIN login, getCurrentUser va requireLogin tra dung User");
    check(!f.auth.login("admin", "wrong") && !f.auth.isLoggedIn(),
          "Login that bai xoa phien cu");
    check(f.auth.login("admin", "secret") &&
          f.auth.login("staff", "secret") &&
          f.auth.getCurrentUser() == f.users.findById("USR0002"),
          "Chuyen phien ADMIN sang STAFF");
    f.auth.logout();
    f.auth.logout();
    check(!f.auth.isLoggedIn() && f.auth.getCurrentUser() == nullptr &&
          throwsError([&] { f.auth.requireLogin(); }),
          "Logout lap va requireLogin khi chua dang nhap");
}

void testPermissionOnly()
{
    cout << "\n===== TEST PERMISSION-ONLY =====" << endl;
    Fixture f("STAFF|MOVIE_VIEW\n");
    f.users.load();
    check(throwsError([&] { f.auth.requirePermission("MOVIE_VIEW"); }),
          "requirePermission chua login nem exception");
    check(f.auth.login("admin", "secret") &&
          !f.auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { f.auth.requirePermission("MOVIE_VIEW"); }),
          "Bang quyen chua load tu choi nghiep vu");
    check(f.permissions.load(), "Nap bang quyen STAFF");
    check(!f.auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { f.auth.requirePermission("MOVIE_VIEW"); }) &&
          &f.auth.requireLogin() == f.users.findById("USR0001"),
          "ADMIN khong duoc cap quyen, requireLogin van hop le");
    check(f.auth.login("staff", "secret") &&
          f.auth.hasPermission("MOVIE_VIEW") &&
          &f.auth.requirePermission("MOVIE_VIEW") == f.users.findById("USR0002"),
          "STAFF duoc cap quyen theo file TXT");
    check(!f.auth.hasPermission("") && !f.auth.hasPermission("UNKNOWN") &&
          !f.auth.hasPermission("MOVIE_ADD") &&
          throwsError([&] { f.auth.requirePermission("UNKNOWN"); }),
          "Quyen rong, ma la va quyen thieu bi tu choi");
    f.auth.logout();
    check(!f.auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { f.auth.requirePermission("MOVIE_VIEW"); }),
          "Logout thu hoi quyen phien");

    Fixture empty;
    empty.users.load();
    empty.permissions.load();
    empty.auth.login("admin", "secret");
    check(!empty.auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { empty.auth.requirePermission("MOVIE_VIEW"); }),
          "Bang quyen rong khong cap quyen");
}

void testReloadAndRevocation()
{
    cout << "\n===== TEST RELOAD / REVOCATION =====" << endl;
    Fixture f("ADMIN|MOVIE_VIEW\n");
    f.users.load();
    f.permissions.load();
    f.auth.login("admin", "secret");
    check(f.auth.hasPermission("MOVIE_VIEW"), "Quyen ban dau duoc cap");
    writeFile(f.files.path("permissions.txt"), "STAFF|MOVIE_VIEW\n");
    check(f.permissions.load() && !f.auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { f.auth.requirePermission("MOVIE_VIEW"); }),
          "Reload thu hoi quyen ADMIN");
    writeFile(f.files.path("permissions.txt"), "ADMIN|MOVIE_VIEW\n");
    check(f.permissions.load() && f.auth.hasPermission("MOVIE_VIEW"),
          "Reload cap lai quyen ADMIN");
    writeFile(f.files.path("permissions.txt"), "ADMIN|UNKNOWN\n");
    check(throwsError([&] { f.permissions.load(); }) &&
          !f.permissions.isLoaded() && !f.auth.hasPermission("MOVIE_VIEW"),
          "Reload sai validation khong dung bang quyen cu");
    writeFile(f.files.path("permissions.txt"), "ADMIN|MOVIE_VIEW\n");
    f.permissions.load();
    fs::remove(f.files.path("permissions.txt"));
    check(!f.permissions.load() && !f.auth.hasPermission("MOVIE_VIEW"),
          "Reload file thieu thu hoi bang quyen cu");
}

void testMockSessionAndHelpers()
{
    cout << "\n===== TEST MOCK / SESSION / HELPERS =====" << endl;
    TestFiles files;
    writeFile(files.path("permissions.txt"), "STAFF|MOVIE_VIEW\n");
    RolePermissionRepository permissions(files.path("permissions.txt").string());
    check(permissions.load(), "Nap bang quyen mock");
    MockUsers concrete;
    IRepository<User>& users = concrete;
    AuthService auth(users, permissions);
    concrete.nullFirst = true;
    check(auth.login("staff", "secret") && concrete.getAtCalls == 2,
          "findByUsername bo qua getAt nullptr");
    concrete.findByIdCalls = 0;
    check(&auth.requirePermission("MOVIE_VIEW") == &concrete.staff &&
          concrete.findByIdCalls == 1,
          "requirePermission tra dung User, chi truy van ID mot lan");
    concrete.staff.setActive(false);
    check(auth.getCurrentUser() == nullptr && !auth.isLoggedIn() &&
          !auth.hasPermission("MOVIE_VIEW") &&
          throwsError([&] { auth.requireLogin(); }),
          "Tai khoan bi vo hieu hoa sau login");
    concrete.staff.setActive(true);
    check(auth.isLoggedIn(), "Kich hoat lai cung ID: phien hop le theo code hien tai");
    concrete.missing = true;
    check(auth.getCurrentUser() == nullptr &&
          throwsError([&] { auth.requirePermission("MOVIE_VIEW"); }),
          "User khong con ton tai sau login");
    concrete.missing = false;
    concrete.loaded = false;
    check(auth.getCurrentUser() == nullptr && !auth.isLoggedIn(),
          "UserRepo khong loaded sau login");
    check(throwsError([&] { auth.login("admin", "secret"); }) &&
          auth.getCurrentUser() == nullptr, "Login chua load xoa phien cu");
    concrete.loaded = true;
    concrete.nullFirst = false;
    concrete.admin.setActive(true);
    concrete.invalidRole = true;
    check(!auth.login("admin", "secret") && !auth.isLoggedIn(),
          "Role khong hop le bi tu choi");
}

int main()
{
    try
    {
        testConstructorAndLogin();
        testPermissionOnly();
        testReloadAndRevocation();
        testMockSessionAndHelpers();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
