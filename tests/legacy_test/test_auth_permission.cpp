
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <string>

#include "../src/business/AuthService.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/RolePermissionRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

// Kiem tra dieu kien
void check(bool condition, const string& testName)
{
    if (condition)
    {
        cout << "[PASS] " << testName << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << testName << endl;
        failed++;
    }
}

// Kiem tra ham co nem runtime_error hay khong
template <typename Func>
bool throwsRuntimeError(Func func)
{
    try
    {
        func();
    }
    catch (const runtime_error&)
    {
        return true;
    }
    return false;
}

// Tao du lieu tai khoan tam
void createUserFile(const string& path)
{
    ofstream file(path);

    file << "USR0001|admin|123456|Admin Test|ADMIN|1\n";
    file << "USR0002|staff|123456|Staff Test|STAFF|1\n";
    file << "USR0003|locked|123456|Locked Test|STAFF|0\n";
}

// Tao du lieu phan quyen tam
void createPermissionFile(const string& path)
{
    ofstream file(path);

    file << "ADMIN|MOVIE_VIEW\n";
    file << "ADMIN|MOVIE_ADD\n";
    file << "ADMIN|MOVIE_DELETE\n";
    file << "STAFF|MOVIE_VIEW\n";
    file << "STAFF|TICKET_SELL\n";
}

int main()
{
    const string userPath = "test_auth_users_temp.txt";
    const string permissionPath = "test_auth_permissions_temp.txt";

    createUserFile(userPath);
    createPermissionFile(permissionPath);

    UserRepository userRepo(userPath);
    RolePermissionRepository permissionRepo(permissionPath);

    // 1. Load du lieu
    check(userRepo.load(), "Load UserRepository");
    check(permissionRepo.load(), "Load RolePermissionRepository");

    AuthService auth(userRepo, permissionRepo);

    // 2. Chua dang nhap
    check(!auth.isLoggedIn(), "Ban dau chua dang nhap");
    check(auth.getCurrentUser() == nullptr, "Chua co current user");
    check(!auth.isAdmin(), "Chua dang nhap khong phai Admin");
    check(!auth.isStaff(), "Chua dang nhap khong phai Staff");
    check(!auth.hasPermission("MOVIE_VIEW"), "Chua dang nhap khong co quyen");

    check(
        throwsRuntimeError([&]() {
            auth.requirePermission("MOVIE_VIEW");
        }),
        "Chua dang nhap requirePermission nem loi"
    );

    // 3. Dang nhap Admin
    check(auth.login("admin", "123456"), "Admin dang nhap thanh cong");
    check(auth.isLoggedIn(), "Admin da dang nhap");
    check(auth.isAdmin(), "Nhan dien vai tro Admin");
    check(!auth.isStaff(), "Admin khong phai Staff");

    check(auth.hasPermission("MOVIE_VIEW"), "Admin co MOVIE_VIEW");
    check(auth.hasPermission("MOVIE_ADD"), "Admin co MOVIE_ADD");
    check(auth.hasPermission("MOVIE_DELETE"), "Admin co MOVIE_DELETE");
    check(!auth.hasPermission("TICKET_SELL"), "Admin khong duoc cap TICKET_SELL");
    check(!auth.hasPermission("UNKNOWN"), "Admin khong co quyen UNKNOWN");

    check(
        auth.requirePermission("MOVIE_ADD").getUserId() == "USR0001",
        "Admin requirePermission thanh cong"
    );

    check(
        throwsRuntimeError([&]() {
            auth.requirePermission("TICKET_SELL");
        }),
        "Admin bi tu choi quyen chua duoc cap"
    );

    // 4. Dang nhap Staff
    check(auth.login("staff", "123456"), "Staff dang nhap thanh cong");
    check(auth.isStaff(), "Nhan dien vai tro Staff");
    check(!auth.isAdmin(), "Staff khong phai Admin");

    check(auth.hasPermission("MOVIE_VIEW"), "Staff co MOVIE_VIEW");
    check(auth.hasPermission("TICKET_SELL"), "Staff co TICKET_SELL");
    check(!auth.hasPermission("MOVIE_ADD"), "Staff khong co MOVIE_ADD");
    check(!auth.hasPermission("MOVIE_DELETE"), "Staff khong co MOVIE_DELETE");

    check(
        auth.requirePermission("TICKET_SELL").getUserId() == "USR0002",
        "Staff requirePermission thanh cong"
    );

    check(
        throwsRuntimeError([&]() {
            auth.requirePermission("MOVIE_ADD");
        }),
        "Staff requirePermission tu choi MOVIE_ADD"
    );

    // 5. Dang nhap sai va dang xuat
    check(!auth.login("staff", "wrong"), "Sai mat khau");
    check(!auth.isLoggedIn(), "Dang nhap sai xoa phien cu");

    check(!auth.login("unknown", "123456"), "Tai khoan khong ton tai");
    check(!auth.login("", "123456"), "Username rong");
    check(!auth.login("staff", ""), "Password rong");

    check(auth.login("staff", "123456"), "Staff dang nhap lai");
    auth.logout();

    check(!auth.isLoggedIn(), "Dang xuat thanh cong");
    check(!auth.hasPermission("TICKET_SELL"), "Dang xuat mat quyen");

    // 6. Tai khoan bi khoa
    check(!auth.login("locked", "123456"), "Tai khoan bi khoa khong dang nhap");

    check(auth.login("staff", "123456"), "Staff dang nhap truoc khi bi khoa");

    User* staff = userRepo.findById("USR0002");
    check(staff != nullptr, "Tim thay Staff");

    if (staff != nullptr)
    {
        staff->setActive(false);

        check(!auth.isLoggedIn(), "Tai khoan bi khoa mat phien hop le");
        check(!auth.hasPermission("MOVIE_VIEW"), "Tai khoan bi khoa mat quyen");

        check(
            throwsRuntimeError([&]() {
                auth.requirePermission("MOVIE_VIEW");
            }),
            "Tai khoan bi khoa requirePermission nem loi"
        );

        staff->setActive(true);
    }

    auth.logout();

    // 7. PermissionRepository chua load
    RolePermissionRepository unloadedRepo(permissionPath);
    AuthService authUnloaded(userRepo, unloadedRepo);

    check(
        authUnloaded.login("admin", "123456"),
        "Van dang nhap duoc khi permission chua load"
    );

    check(
        !authUnloaded.hasPermission("MOVIE_ADD"),
        "Permission chua load thi tu choi"
    );

    check(
        throwsRuntimeError([&]() {
            authUnloaded.requirePermission("MOVIE_ADD");
        }),
        "RequirePermission nem loi khi permission chua load"
    );

    // 8. Cap nhat quyen qua file TXT
    check(auth.login("staff", "123456"), "Staff dang nhap de test cap quyen");
    check(!auth.hasPermission("MOVIE_ADD"), "Truoc khi cap quyen Staff khong co MOVIE_ADD");

    {
        ofstream file(permissionPath);

        file << "ADMIN|MOVIE_VIEW\n";
        file << "ADMIN|MOVIE_ADD\n";
        file << "STAFF|MOVIE_VIEW\n";
        file << "STAFF|MOVIE_ADD\n";
    }

    check(permissionRepo.load(), "Load lai bang phan quyen");
    check(auth.hasPermission("MOVIE_ADD"), "Staff nhan MOVIE_ADD sau khi load lai");
    check(!auth.hasPermission("TICKET_SELL"), "Staff mat TICKET_SELL sau khi load lai");

    check(
        auth.requirePermission("MOVIE_ADD").getUserId() == "USR0002",
        "Staff su dung quyen moi thanh cong"
    );

    // 9. Tong ket
    auth.logout();
    authUnloaded.logout();

    remove(userPath.c_str());
    remove(permissionPath.c_str());

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
