
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include "../src/business/MemberService.h"

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

void writeFile(const string& path, const string& content)
{
    ofstream file(path, ios::trunc);
    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test: " + path);
    }
    file << content;
    file.close();
    if (!file)
    {
        throw runtime_error("Khong ghi duoc file test: " + path);
    }
}

bool addMemberSafe(MemberService& service, Member* member)
{
    try
    {
        bool result = service.addMember(member);
        if (!result)
        {
            delete member;
        }
        return result;
    }
    catch (...)
    {
        delete member;
        throw;
    }
}

int main()
{
    const string memberPath = "test_member_service_members.txt";
    const string userPath = "test_member_service_users.txt";
    const string permissionPath = "test_member_service_permissions.txt";

    try
    {
        writeFile(memberPath,
            "MEM0001|Nguyen Van A|0901234567|100\n"
            "MEM0002|Tran Thi B|0912345678|50\n"
        );

        writeFile(userPath,
            "USR0001|admin01|123456|Admin One|ADMIN|1\n"
            "USR0002|staff01|123456|Staff One|STAFF|1\n"
            "USR0003|staff02|123456|Staff Two|STAFF|0\n"
        );

        // Admin co du 3 quyen.
        // Staff chi co VIEW va ADD, khong co UPDATE.
        writeFile(permissionPath,
            "ADMIN|MEMBER_VIEW\n"
            "ADMIN|MEMBER_ADD\n"
            "ADMIN|MEMBER_UPDATE\n"
            "STAFF|MEMBER_VIEW\n"
            "STAFF|MEMBER_ADD\n"
        );

        {
            UserRepository userRepo(userPath);
            MemberRepository memberRepo(memberPath);
            RolePermissionRepository permissionRepo(permissionPath);

            AuthService authService(userRepo, permissionRepo);
            MemberService memberService(memberRepo, authService);

            // =====================================
            // NHOM 1: LOAD VA SAVE
            // =====================================

            check(throwsRuntimeError([&]() {
                memberService.save();
            }), "Save before load throws");

            check(memberService.load(), "Load members successfully");
            check(memberRepo.isLoaded(), "Member repository loaded");

            check(throwsRuntimeError([&]() {
                memberService.load();
            }), "Cannot load twice");

            check(memberService.save(), "Save members successfully");

            check(userRepo.load(), "Load users");
            check(permissionRepo.load(), "Load permissions");

            // =====================================
            // NHOM 2: CHUA DANG NHAP
            // =====================================

            check(throwsRuntimeError([&]() {
                memberService.getMemberCount();
            }), "Guest cannot count members");

            check(throwsRuntimeError([&]() {
                memberService.findMemberById("MEM0001");
            }), "Guest cannot find by ID");

            check(throwsRuntimeError([&]() {
                memberService.findMemberByPhone("0901234567");
            }), "Guest cannot find by phone");

            check(throwsRuntimeError([&]() {
                memberService.getMemberAt(0);
            }), "Guest cannot get member at index");

            check(throwsRuntimeError([&]() {
                memberService.addMember(nullptr);
            }), "Guest cannot add member");

            check(throwsRuntimeError([&]() {
                memberService.updateMember(
                    Member("MEM0001", "Changed", "0901234567", 100)
                );
            }), "Guest cannot update member");

            // =====================================
            // NHOM 3: PHAN QUYEN STAFF
            // =====================================

            check(authService.login("staff01", "123456"),
                  "Staff login successfully");

            check(memberService.getMemberCount() == 2,
                  "Staff can view member count");

            check(memberService.findMemberById("MEM0001") != nullptr,
                  "Staff can find member");

            check(throwsRuntimeError([&]() {
                memberService.updateMember(
                    Member("MEM0001", "Changed", "0901234567", 100)
                );
            }), "Staff without UPDATE permission is denied");

            check(addMemberSafe(memberService,
                new Member("MEM0003", "Le Van C", "0987654321", 0)
            ), "Staff can add member");

            check(memberService.getMemberCount() == 3,
                  "Count after staff adds member");

            authService.logout();

            check(throwsRuntimeError([&]() {
                memberService.getMemberCount();
            }), "Cannot view after logout");

            check(!authService.login("staff02", "123456"),
                  "Inactive staff cannot login");

            // =====================================
            // NHOM 4: ADMIN
            // =====================================

            check(authService.login("admin01", "123456"),
                  "Admin login successfully");

            check(memberService.getMemberCount() == 3,
                  "Admin can count members");

            // =====================================
            // NHOM 5: ADD MEMBER
            // =====================================

            check(!memberService.addMember(nullptr),
                  "Reject nullptr");

            check(addMemberSafe(memberService,
                new Member("MEM0004", "Pham Van D", "0976543210", 0)
            ), "Add valid member");

            check(!addMemberSafe(memberService,
                new Member("MEM0004", "Duplicate ID", "0965432109", 0)
            ), "Reject duplicate ID");

            check(!addMemberSafe(memberService,
                new Member("MEM0005", "Duplicate Phone", "0976543210", 0)
            ), "Reject duplicate phone");

            check(!addMemberSafe(memberService,
                new Member("BAD0006", "Invalid ID", "0954321098", 0)
            ), "Reject invalid ID");

            check(!addMemberSafe(memberService,
                new Member("MEM0006", "   ", "0954321098", 0)
            ), "Reject invalid name");

            check(!addMemberSafe(memberService,
                new Member("MEM0006", "Invalid Phone", "123", 0)
            ), "Reject invalid phone");

            check(!addMemberSafe(memberService,
                new Member("MEM0006", "Negative Points", "0954321098", -5)
            ), "Reject negative points");

            check(!addMemberSafe(memberService,
                new Member("MEM0006", "Nonzero Points", "0954321098", 100)
            ), "Reject initial points not zero");

            check(memberService.getMemberCount() == 4,
                  "Invalid adds do not change count");

            // =====================================
            // NHOM 6: UPDATE MEMBER
            // =====================================

            check(memberService.updateMember(
                Member("MEM0001", "Nguyen Van Updated", "0999999999", 100)
            ), "Update member successfully");

            const Member* updated =
                memberService.findMemberById("MEM0001");

            check(updated != nullptr &&
                  updated->getFullName() == "Nguyen Van Updated" &&
                  updated->getPhone() == "0999999999" &&
                  updated->getPoints() == 100,
                  "Updated information is correct");

            check(memberService.findMemberByPhone("0901234567") == nullptr,
                  "Old phone removed from index");

            check(memberService.findMemberByPhone("0999999999") == updated,
                  "New phone indexed correctly");

            check(!memberService.updateMember(
                Member("MEM9999", "Missing", "0943210987", 0)
            ), "Reject nonexistent member");

            check(!memberService.updateMember(
                Member("MEM0001", "Duplicate Phone", "0912345678", 100)
            ), "Reject duplicate phone on update");

            check(!memberService.updateMember(
                Member("MEM0001", "Invalid Phone", "123", 100)
            ), "Reject invalid phone on update");

            check(!memberService.updateMember(
                Member("MEM0001", "Changed Points", "0999999999", 200)
            ), "Reject changing points");

            updated = memberService.findMemberById("MEM0001");

            check(updated != nullptr &&
                  updated->getPoints() == 100 &&
                  updated->getFullName() == "Nguyen Van Updated",
                  "Failed update preserves member data");

            check(memberService.updateMember(
                Member("MEM0001", "Name Only", "0999999999", 100)
            ), "Update name without changing phone");

            // =====================================
            // NHOM 7: TRA CUU
            // =====================================

            check(memberService.findMemberById("MEM0002") != nullptr,
                  "Find existing ID");

            check(memberService.findMemberById("MEM9999") == nullptr,
                  "Missing ID returns nullptr");

            check(memberService.findMemberByPhone("0912345678") != nullptr,
                  "Find existing phone");

            check(memberService.findMemberByPhone("0123456789") == nullptr,
                  "Missing phone returns nullptr");

            check(memberService.getMemberAt(0) != nullptr,
                  "Get first member");

            check(memberService.getMemberAt(3) != nullptr,
                  "Get last member");

            check(memberService.getMemberAt(-1) == nullptr,
                  "Reject negative index");

            check(memberService.getMemberAt(4) == nullptr,
                  "Reject out-of-range index");

            check(memberService.getMemberCount() == 4,
                  "Final member count");

            // =====================================
            // NHOM 8: SAVE
            // =====================================

            check(memberService.save(), "Save modified member data");
        }

        // =====================================
        // NHOM 9: RELOAD
        // =====================================

        {
            UserRepository userRepo(userPath);
            MemberRepository memberRepo(memberPath);
            RolePermissionRepository permissionRepo(permissionPath);

            AuthService authService(userRepo, permissionRepo);
            MemberService memberService(memberRepo, authService);

            check(userRepo.load(), "Reload users");
            check(permissionRepo.load(), "Reload permissions");
            check(memberService.load(), "Reload members");

            check(authService.login("admin01", "123456"),
                  "Admin login after reload");

            check(memberService.getMemberCount() == 4,
                  "Member count persists");

            const Member* member =
                memberService.findMemberById("MEM0001");

            check(member != nullptr &&
                  member->getFullName() == "Name Only" &&
                  member->getPhone() == "0999999999" &&
                  member->getPoints() == 100,
                  "Updated member persists");

            check(memberService.findMemberById("MEM0004") != nullptr,
                  "Added member persists");
        }

        // =====================================
        // NHOM 10: ADMIN THIEU QUYEN
        // =====================================

        writeFile(permissionPath, "ADMIN|MEMBER_VIEW\n");

        {
            UserRepository userRepo(userPath);
            MemberRepository memberRepo(memberPath);
            RolePermissionRepository permissionRepo(permissionPath);

            check(userRepo.load(), "Load users for restricted Admin");
            check(permissionRepo.load(), "Load restricted permissions");
            check(memberRepo.load(), "Load members for restricted Admin");

            AuthService authService(userRepo, permissionRepo);
            MemberService memberService(memberRepo, authService);

            check(authService.login("admin01", "123456"),
                  "Restricted Admin login");

            check(memberService.getMemberCount() == 4,
                  "Restricted Admin can view");

            check(throwsRuntimeError([&]() {
                memberService.addMember(nullptr);
            }), "Admin without MEMBER_ADD is denied");

            check(throwsRuntimeError([&]() {
                memberService.updateMember(
                    Member("MEM0001", "Changed", "0999999999", 100)
                );
            }), "Admin without MEMBER_UPDATE is denied");
        }

        // =====================================
        // NHOM 11: LOI DU LIEU
        // =====================================

        writeFile(memberPath,
            "MEM0001|Nguyen Van A|0901234567\n"
        );

        {
            MemberRepository memberRepo(memberPath);
            UserRepository userRepo(userPath);
            RolePermissionRepository permissionRepo(permissionPath);
            AuthService authService(userRepo, permissionRepo);
            MemberService memberService(memberRepo, authService);

            check(throwsRuntimeError([&]() {
                memberService.load();
            }), "Invalid member file throws");

            check(!memberRepo.isLoaded(),
                  "Invalid load does not mark repository loaded");
        }
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }
    catch (...)
    {
        failed++;
        cout << "[ERROR] Unknown exception" << endl;
    }

    remove(memberPath.c_str());
    remove(userPath.c_str());
    remove(permissionPath.c_str());

    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
