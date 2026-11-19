
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include "../src/business/StaffService.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    if (condition)
    {
        passed++;
        cout << "[PASS] " << name << endl;
    }
    else
    {
        failed++;
        cout << "[FAIL] " << name << endl;
    }
}

// Kiem tra ham co nem ngoai le hay khong
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

// Tao file du lieu test
void createTestFile(const string& path)
{
    ofstream file(path, ios::trunc);

    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }

    file << "USR0001|admin01|123456|Admin One|ADMIN|1\n";
    file << "USR0002|staff01|123456|Staff One|STAFF|1\n";
    file << "USR0003|staff02|123456|Staff Two|STAFF|1\n";
    file << "USR0004|staff03|123456|Staff Three|STAFF|0\n";
}

// Them User, neu that bai thi giai phong con tro
bool addStaffSafe(StaffService& service, User* user)
{
    try
    {
        bool result = service.addStaff(user);

        if (!result)
        {
            delete user;
        }

        return result;
    }
    catch (...)
    {
        delete user;
        throw;
    }
}

int main()
{
    const string path = "test_staff_service_data.txt";
    const string permissionPath = "test_staff_service_permissions.txt";

    try
    {
        createTestFile(path);
        {
            ofstream permissions(permissionPath, ios::trunc);
            permissions << "ADMIN|STAFF_VIEW\nADMIN|STAFF_ADD\n"
                        << "ADMIN|STAFF_UPDATE\nADMIN|STAFF_DEACTIVATE\n";
        }

        {
            UserRepository staffRepo(path);
            RolePermissionRepository permissionRepo(permissionPath);
            check(permissionRepo.load(), "Load staff permissions");
            AuthService authService(staffRepo, permissionRepo);
            StaffService staffService(staffRepo, authService);

            // ======================================
            // NHOM 1: LOAD VA SAVE
            // ======================================

            check(throwsRuntimeError([&]() {
            staffService.save();
            }), "Save before load throws runtime_error");

            check(staffService.load(),
                  "Load successfully");

            check(staffRepo.isLoaded(),
                  "Repository is loaded");

            check(throwsRuntimeError([&]() {
            staffService.load();
            }), "Cannot load twice");

            check(staffService.save(),
                  "Save successfully");

            // ======================================
            // NHOM 2: PHAN QUYEN
            // ======================================

            check(throwsRuntimeError([&]() {
                staffService.getStaffCount();
            }), "Guest cannot count staff");

            check(throwsRuntimeError([&]() {
                staffService.findStaffById("USR0002");
            }), "Guest cannot find staff");

            check(throwsRuntimeError([&]() {
                staffService.addStaff(nullptr);
            }), "Guest cannot add staff");

            check(throwsRuntimeError([&]() {
                staffService.deactivateStaff("USR0002");
            }), "Guest cannot deactivate staff");

            check(authService.login("staff01", "123456"),
                  "Staff login successfully");

            check(throwsRuntimeError([&]() {
                staffService.getStaffCount();
            }), "Staff cannot count staff");

            check(throwsRuntimeError([&]() {
                staffService.findStaffByUsername("staff02");
            }), "Staff cannot search staff");

            check(throwsRuntimeError([&]() {
                staffService.updateStaff(
                    User("USR0002", "staff01", "123456",
                         "Changed", UserRole::STAFF, true)
                );
            }), "Staff cannot update staff");

            check(throwsRuntimeError([&]() {
                staffService.deactivateStaff("USR0003");
            }), "Staff cannot deactivate another staff");

            authService.logout();

            check(authService.login("admin01", "123456"),
                  "Admin login successfully");

            check(authService.isAdmin(),
                  "Current user is Admin");

            // ======================================
            // NHOM 3: THEM NHAN VIEN
            // ======================================

            check(!staffService.addStaff(nullptr),
                  "Reject null staff");

            check(!addStaffSafe(
                staffService,
                new User("USR0005", "admin02", "123456",
                         "Admin Two", UserRole::ADMIN, true)
            ), "Reject adding Admin");

            check(addStaffSafe(
                staffService,
                new User("USR0005", "staff04", "123456",
                         "Staff Four", UserRole::STAFF, true)
            ), "Add valid staff");

            check(staffService.getStaffCount() == 4,
                  "Count increases after add");

            check(!addStaffSafe(
                staffService,
                new User("USR0005", "staff05", "123456",
                         "Duplicate ID", UserRole::STAFF, true)
            ), "Reject duplicate ID");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", "staff04", "123456",
                         "Duplicate Username", UserRole::STAFF, true)
            ), "Reject duplicate username");

            check(!addStaffSafe(
                staffService,
                new User("BAD0001", "staff06", "123456",
                         "Invalid ID", UserRole::STAFF, true)
            ), "Reject invalid ID");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", "", "123456",
                         "Invalid Username", UserRole::STAFF, true)
            ), "Reject empty username");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", " staff06", "123456",
                         "Invalid Username", UserRole::STAFF, true)
            ), "Reject username with leading space");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", "staff06", "",
                         "Invalid Password", UserRole::STAFF, true)
            ), "Reject empty password");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", "staff06", "123456",
                         "", UserRole::STAFF, true)
            ), "Reject empty full name");

            check(!addStaffSafe(
                staffService,
                new User("USR0006", "staff|06", "123456",
                         "Invalid Character", UserRole::STAFF, true)
            ), "Reject delimiter in username");

            // ======================================
            // NHOM 4: CAP NHAT NHAN VIEN
            // ======================================

            check(staffService.updateStaff(
                User("USR0002", "staff01new", "654321",
                     "Staff One Updated", UserRole::STAFF, true)
            ), "Update valid staff");

            const User* updated =
                staffService.findStaffById("USR0002");

            check(updated != nullptr &&
                  updated->getUsername() == "staff01new" &&
                  updated->getPassword() == "654321" &&
                  updated->getFullName() == "Staff One Updated",
                  "Updated information is correct");

            check(staffService.findStaffByUsername("staff01") == nullptr,
                  "Old username is removed from index");

            check(staffService.findStaffByUsername("staff01new") != nullptr,
                  "New username is searchable");

            check(!staffService.updateStaff(
                User("USR9999", "missing", "123456",
                     "Missing", UserRole::STAFF, true)
            ), "Reject updating nonexistent staff");

            check(!staffService.updateStaff(
                User("USR0001", "admin01", "123456",
                     "Changed Admin", UserRole::ADMIN, true)
            ), "Reject updating Admin");

            check(!staffService.updateStaff(
                User("USR0002", "staff01new", "654321",
                     "Changed Role", UserRole::ADMIN, true)
            ), "Reject changing role");

            check(!staffService.updateStaff(
                User("USR0002", "staff02", "654321",
                     "Duplicate Username", UserRole::STAFF, true)
            ), "Reject duplicate username on update");

            check(!staffService.updateStaff(
                User("USR0002", "staff01new", "654321",
                     "Changed Active", UserRole::STAFF, false)
            ), "Reject changing active through update");

            check(staffService.findStaffById("USR0002")->isActive(),
                  "Failed update preserves active status");

            check(staffService.updateStaff(
                User("USR0004", "staff03", "123456",
                     "Inactive Staff Updated", UserRole::STAFF, false)
            ), "Update inactive staff information");

            // ======================================
            // NHOM 5: VO HIEU HOA NHAN VIEN
            // ======================================

            check(staffService.deactivateStaff("USR0003"),
                  "Deactivate active staff");

            const User* inactive =
                staffService.findStaffById("USR0003");

            check(inactive != nullptr && !inactive->isActive(),
                  "Staff becomes inactive");

            check(!staffService.deactivateStaff("USR0003"),
                  "Reject deactivating inactive staff twice");

            check(!staffService.deactivateStaff("USR0001"),
                  "Reject deactivating Admin");

            check(!staffService.deactivateStaff("USR9999"),
                  "Reject deactivating nonexistent staff");

            check(staffService.getStaffCount() == 4,
                  "Inactive staff is still counted");

            // ======================================
            // NHOM 6: TRA CUU NHAN VIEN
            // ======================================

            check(staffService.findStaffById("USR0002") != nullptr,
                  "Find staff by ID");

            check(staffService.findStaffById("USR0001") == nullptr,
                  "Do not return Admin by ID");

            check(staffService.findStaffById("USR9999") == nullptr,
                  "Nonexistent ID returns nullptr");

            check(staffService.findStaffByUsername("staff02") != nullptr,
                  "Find inactive staff by username");

            check(staffService.findStaffByUsername("admin01") == nullptr,
                  "Do not return Admin by username");

            check(staffService.findStaffByUsername("missing") == nullptr,
                  "Nonexistent username returns nullptr");

            check(staffService.getStaffAt(0) != nullptr &&
                  staffService.getStaffAt(0)->getUserId() == "USR0002",
                  "Get first staff");

            check(staffService.getStaffAt(1) != nullptr &&
                  staffService.getStaffAt(1)->getUserId() == "USR0003",
                  "Get second staff");

            check(staffService.getStaffAt(3) != nullptr &&
                  staffService.getStaffAt(3)->getUserId() == "USR0005",
                  "Get last staff");

            check(staffService.getStaffAt(-1) == nullptr,
                  "Reject negative index");

            check(staffService.getStaffAt(4) == nullptr,
                  "Reject out-of-range index");

            check(staffService.getStaffCount() == 4,
                  "Count only STAFF accounts");

            // ======================================
            // NHOM 7: SAVE VA RELOAD
            // ======================================

            check(staffService.save(),
                  "Save modified staff data");
        }

        // Repository cu da duoc huy.
        // Tao Repository moi de kiem tra du lieu tren file.
        {
            UserRepository staffRepo(path);
            RolePermissionRepository permissionRepo(permissionPath);
            check(permissionRepo.load(), "Reload staff permissions");
            AuthService authService(staffRepo, permissionRepo);
            StaffService staffService(staffRepo, authService);

            check(staffService.load(),
                  "Reload saved data");

            check(authService.login("admin01", "123456"),
                  "Admin login after reload");

            check(staffService.getStaffCount() == 4,
                  "Staff count persists after reload");

            const User* staff =
                staffService.findStaffById("USR0002");

            check(staff != nullptr &&
                  staff->getUsername() == "staff01new" &&
                  staff->getFullName() == "Staff One Updated",
                  "Updated staff persists after reload");

            const User* disabled =
                staffService.findStaffById("USR0003");

            check(disabled != nullptr && !disabled->isActive(),
                  "Inactive status persists after reload");

            check(staffService.findStaffById("USR0005") != nullptr,
                  "Added staff persists after reload");

            authService.logout();

            check(!authService.login("staff02", "123456"),
                  "Inactive staff cannot log in");

            check(!authService.login("staff01", "123456"),
                  "Old username cannot log in");

            check(authService.login("staff01new", "654321"),
                  "Updated credentials can log in");
        }
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }

    // Xoa file du lieu test
    remove(path.c_str());
    remove(permissionPath.c_str());

    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
