#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "../src/business/MemberService.h"
#include "../src/data_access/MemberRepository.h"
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
    ofstream out(path, ios::binary | ios::trunc);
    if (!out || !(out << data)) throw runtime_error("Khong ghi duoc TXT test");
}

string readFile(const fs::path& path)
{
    ifstream in(path, ios::binary);
    return string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
}

struct TestFiles
{
    fs::path dir;
    TestFiles()
    {
        dir = fs::temp_directory_path() /
              ("pbl2_member_service_" +
               to_string(chrono::steady_clock::now().time_since_epoch().count()));
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
    "ADMIN|MEMBER_VIEW\nADMIN|MEMBER_ADD\nADMIN|MEMBER_UPDATE\n"
    "STAFF|MEMBER_VIEW\nSTAFF|MEMBER_ADD\nSTAFF|MEMBER_UPDATE\n";
const string MEMBERS =
    "MEM0002|First Member|0901234567|10\n"
    "MEM0005|Second Member|0912345678|0\n";

struct Fixture
{
    TestFiles files;
    MemberRepository concrete;
    IRepository<Member>& members;
    UserRepository concreteUsers;
    IRepository<User>& users;
    RolePermissionRepository permissions;
    AuthService auth;
    MemberService service;

    Fixture(const string& data = MEMBERS, const string& rights = RIGHTS)
        : concrete(files.path("members.txt").string()), members(concrete),
          concreteUsers(files.path("users.txt").string()), users(concreteUsers),
          permissions(files.path("rights.txt").string()),
          auth(users, permissions), service(members, auth)
    {
        writeFile(files.path("members.txt"), data);
        writeFile(files.path("users.txt"), USERS);
        writeFile(files.path("rights.txt"), rights);
        if (!permissions.load()) throw runtime_error("Khong nap duoc quyen test");
    }
    void login(const string& name = "admin")
    {
        if (!users.isLoaded() && !users.load())
            throw runtime_error("Khong nap duoc user test");
        if (!auth.login(name, "secret")) throw runtime_error("Login test that bai");
    }
};

struct MockMembers : IRepository<Member>
{
    vector<Member*> items;
    bool loaded = true;
    bool failAdd = false;
    bool throwAdd = false;
    int addCalls = 0;
    int updateCalls = 0;
    ~MockMembers() { for (Member* item : items) delete item; }
    bool load() override { loaded = true; return true; }
    bool save() override { return loaded; }
    bool add(Member* item) override
    {
        addCalls++;
        if (throwAdd) throw runtime_error("Mock add exception");
        if (failAdd) return false;
        items.push_back(item);
        return true;
    }
    bool update(const Member&) override { updateCalls++; return true; }
    bool removeById(const string&) override { return false; }
    Member* findById(const string& id) override
    {
        for (Member* item : items) if (item->getMemberId() == id) return item;
        return nullptr;
    }
    Member* getAt(int index) override
    {
        return index >= 0 && index < static_cast<int>(items.size()) ? items[index] : nullptr;
    }
    int count() override { return static_cast<int>(items.size()); }
    bool isLoaded() override { return loaded; }
};

void testLoadSaveAndRollback()
{
    Fixture f;
    check(!f.service.save(), "Save truoc load tra false");
    check(f.service.load(), "Load TXT thanh cong");
    check(!f.service.load() && f.concrete.findByPhone("0901234567") != nullptr,
          "Load lan hai tra false va giu chi muc phone");
    check(f.service.save() && readFile(f.files.path("members.txt")).find("MEM0005") != string::npos,
          "Save TXT thanh cong");

    Fixture bad("MEM0001|One Member|0901234567|0\n"
                "MEM0002|Two Member|0901234567|0\n");
    check(throwsError([&] { bad.service.load(); }), "Load trung phone nem exception");
    check(!bad.members.isLoaded() && bad.members.count() == 0 &&
          bad.concrete.findByPhone("0901234567") == nullptr,
          "Load loi rollback danh sach va chi muc phone");
    writeFile(bad.files.path("members.txt"), "MEM0001|One Member|0901234567|0\n");
    check(bad.service.load() && bad.members.count() == 1 &&
          bad.concrete.findByPhone("0901234567") != nullptr,
          "Sua TXT loi va load lai thanh cong");
}

void testAddAndUpdate()
{
    Fixture f;
    f.login();
    check(!f.service.addMember(nullptr), "Add nullptr tra false");
    Member before("MEM0999", "Input Member", "0923456789", 0);
    check(!f.service.addMember(&before), "Add khi Repository chua load tra false");
    check(f.service.load(), "Load truoc thao tac add");
    check(f.service.addMember(&before), "Add thanh vien hop le");
    const Member* added = f.service.findMemberById("MEM0006");
    check(added != nullptr && added->getPhone() == "0923456789",
          "Sinh ID theo ID lon nhat");
    check(before.getMemberId() == "MEM0999" && before.getPhone() == "0923456789",
          "Khong thay doi doi tuong dau vao");
    Member duplicate("", "Duplicate", "0923456789", 0);
    Member badPhone("", "Bad Phone", "abc", 0);
    Member badName("", "", "0934567890", 0);
    Member badPoints("", "Bad Points", "0934567890", 2);
    check(!f.service.addMember(&duplicate), "Tu choi phone trung");
    check(!f.service.addMember(&badPhone), "Tu choi phone sai");
    check(!f.service.addMember(&badName), "Tu choi ten rong");
    check(!f.service.addMember(&badPoints), "Tu choi diem khoi tao khac 0");
    check(f.service.getMemberCount() == 3, "Add that bai giu nguyen so luong");

    Member* original = f.concrete.findById("MEM0002");
    Invoice invoice("INV0001", "SHW0001", "USR0001", "MEM0002",
                    "2026-11-27 10:00", 100000);
    check(invoice.linkMember(original) && original->getInvoiceCount() == 1,
          "Lien ket Invoice truoc update");
    Member changed("MEM0002", "Updated Member", "0945678901", 10);
    check(f.service.updateMember(changed), "Update ten va phone hop le");
    check(f.concrete.findById("MEM0002") == original &&
          original->getMemberId() == "MEM0002" && original->getInvoiceCount() == 1 &&
          invoice.getMember() == original, "Update giu ID, dia chi va Invoice");
    check(f.service.findMemberByPhone("0901234567") == nullptr &&
          f.service.findMemberByPhone("0945678901") == original,
          "Tra cuu phone cu/moi sau update");
    Member points("MEM0002", "Updated Member", "0945678901", 11);
    Member conflict("MEM0002", "Updated Member", "0912345678", 10);
    Member absent("MEM9999", "Absent Member", "0956789012", 0);
    check(!f.service.updateMember(points) && original->getPoints() == 10,
          "Khong cho thay doi diem truc tiep");
    check(!f.service.updateMember(conflict), "Update phone trung bi tu choi");
    check(!f.service.updateMember(absent), "Update ID khong ton tai bi tu choi");
    check(!f.service.updateMember(badPhone), "Update du lieu sai bi tu choi");
    invoice.unlinkReferences();
}

void testQueriesAndPermission()
{
    Fixture f;
    check(throwsError([&] { f.service.getMemberCount(); }) &&
          throwsError([&] { f.service.addMember(nullptr); }),
          "Chua dang nhap: VIEW va ADD nem exception");
    f.login("staff");
    check(f.service.load() && f.service.getMemberCount() == 2,
          "STAFF duoc cap quyen load va count");
    check(f.service.findMemberById("MEM0002") == f.service.getMemberAt(0),
          "Tim ID va truy xuat index hop le");
    check(f.service.findMemberByPhone("0912345678") == f.service.getMemberAt(1),
          "Tim theo phone hop le");
    check(f.service.findMemberById("MEM9999") == nullptr &&
          f.service.findMemberByPhone("0999999999") == nullptr,
          "ID va phone khong ton tai");
    check(f.service.getMemberAt(-1) == nullptr && f.service.getMemberAt(2) == nullptr,
          "Index am va vuot bien");

    Fixture denied(MEMBERS, "ADMIN|STAFF_VIEW\nSTAFF|MEMBER_VIEW\n");
    denied.login();
    check(denied.service.load(), "ADMIN thieu quyen: load khong can permission");
    Member input("", "New Member", "0923456789", 0);
    Member changed("MEM0002", "Changed Member", "0901234567", 10);
    check(throwsError([&] { denied.service.addMember(&input); }) &&
          throwsError([&] { denied.service.updateMember(changed); }) &&
          throwsError([&] { denied.service.getMemberCount(); }),
          "ADMIN khong mac nhien co ADD, UPDATE, VIEW");
    check(denied.members.count() == 2, "Thieu quyen khong thay doi du lieu");

    Fixture viewOnly(MEMBERS, "STAFF|MEMBER_VIEW\n");
    viewOnly.login("staff");
    check(viewOnly.service.load() && viewOnly.service.getMemberCount() == 2,
          "STAFF duoc cap rieng MEMBER_VIEW");
    check(throwsError([&] { viewOnly.service.addMember(&input); }) &&
          throwsError([&] { viewOnly.service.updateMember(changed); }),
          "MEMBER_VIEW khong cap MEMBER_ADD hoac MEMBER_UPDATE");
    check(!viewOnly.auth.login("off", "secret"),
          "Tai khoan inactive khong the dang nhap");
}

void testMockDIPAndFailure()
{
    static_assert(is_constructible<MemberService, IRepository<Member>&, AuthService&>::value,
                  "MemberService phai nhan IRepository<Member>&");
    Fixture f;
    f.login();
    MockMembers mock;
    MemberService service(mock, f.auth);
    check(service.load() && service.save(), "DIP/DI: mock load va save");
    Member input("", "Mock Member", "0923456789", 0);
    mock.failAdd = true;
    check(!service.addMember(&input) && mock.count() == 0 && mock.addCalls == 1,
          "Mock add false: khong chuyen quyen so huu");
    mock.failAdd = false;
    mock.throwAdd = true;
    check(throwsError([&] { service.addMember(&input); }) && mock.count() == 0,
          "Mock add exception duoc truyen lai");
    mock.throwAdd = false;
    check(service.addMember(&input) && mock.count() == 1 &&
          service.findMemberById("MEM0001") == mock.getAt(0),
          "Mock add thanh cong va service tra con tro muon");
    Member updated("MEM0001", "Updated Member", "0923456789", 0);
    check(service.updateMember(updated) && mock.updateCalls == 1,
          "Mock update duoc dieu phoi qua interface");
    check(service.findMemberByPhone("0923456789") == mock.getAt(0) &&
          service.getMemberAt(0) == mock.getAt(0) && service.getMemberCount() == 1,
          "Mock query qua interface");
    mock.loaded = false;
    check(!service.addMember(&input), "Mock chua load: add tra false");
}

int main()
{
    try
    {
        testLoadSaveAndRollback();
        testAddAndUpdate();
        testQueriesAndPermission();
        testMockDIPAndFailure();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
