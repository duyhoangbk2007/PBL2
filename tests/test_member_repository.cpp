
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "../src/data_access/MemberRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_members_temp.txt";

// Kiem tra ket qua
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

// Tao file du lieu tam
void writeTestFile(const string& content)
{
    ofstream out(TEST_FILE, ios::trunc);

    if (!out.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }

    out << content;
    out.close();

    if (!out)
    {
        throw runtime_error("Khong ghi duoc file test");
    }
}

// Doc noi dung file
string readTestFile()
{
    ifstream in(TEST_FILE);
    string content;
    string line;

    while (getline(in, line))
    {
        content += line + "\n";
    }

    return content;
}

// Test 1: Load file rong
void testLoadEmpty()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);

    check(!repo.isLoaded(), "Chua load ban dau");
    check(repo.load(), "Load file rong");
    check(repo.isLoaded(), "Trang thai sau load");
    check(repo.count() == 0, "So luong ban dau bang 0");
    check(repo.getAt(0) == nullptr, "GetAt file rong");
}

// Test 2: Load du lieu hop le
void testLoadValid()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|100\n"
        "MEM0002|Tran Thi B|0912345678|50\n"
    );

    MemberRepository repo(TEST_FILE);
    repo.load();

    check(repo.count() == 2, "Load 2 thanh vien");

    Member* a = repo.findById("MEM0001");
    Member* b = repo.findByPhone("0912345678");

    check(a != nullptr, "Tim theo ID");
    check(b != nullptr, "Tim theo phone");

    check(a != nullptr && a->getPoints() == 100,
          "Doc dung diem");

    check(b != nullptr && b->getFullName() == "Tran Thi B",
          "Doc dung ho ten");

    check(repo.getAt(0) == a, "GetAt vi tri 0");
    check(repo.getAt(-1) == nullptr, "GetAt vi tri am");
    check(repo.getAt(2) == nullptr, "GetAt vuot pham vi");

    check(repo.findById("MEM9999") == nullptr,
          "Khong tim thay ID");

    check(repo.findByPhone("0999999999") == nullptr,
          "Khong tim thay phone");
}

// Test 3: Khong load hai lan
void testLoadTwice()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load hai lan nem ngoai le");
}

// Test 4: Them thanh vien hop le
void testAddValid()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member* member = new Member(
        "MEM0001", "Nguyen Van A", "0901234567", 0
    );

    bool result = repo.add(member);

    check(result, "Add thanh vien hop le");
    check(repo.count() == 1, "Count sau add");
    check(repo.findById("MEM0001") == member,
          "Tim ID sau add");
    check(repo.findByPhone("0901234567") == member,
          "Tim phone sau add");

    if (!result)
    {
        delete member;
    }
}

// Test 5: Them thanh vien trung ID
void testAddDuplicateId()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member* a = new Member(
        "MEM0001", "Nguyen Van A", "0901234567", 0
    );

    Member* b = new Member(
        "MEM0001", "Tran Van B", "0912345678", 0
    );

    bool first = repo.add(a);
    bool second = repo.add(b);

    check(first && !second, "Tu choi trung ID");
    check(repo.count() == 1, "Count khi trung ID");

    if (!first) delete a;
    if (!second) delete b;
}

// Test 6: Them thanh vien trung phone
void testAddDuplicatePhone()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member* a = new Member(
        "MEM0001", "Nguyen Van A", "0901234567", 0
    );

    Member* b = new Member(
        "MEM0002", "Tran Van B", "0901234567", 0
    );

    bool first = repo.add(a);
    bool second = repo.add(b);

    check(first && !second, "Tu choi trung phone");
    check(repo.count() == 1, "Count khi trung phone");

    if (!first) delete a;
    if (!second) delete b;
}

// Test 7: Kiem tra du lieu add khong hop le
void testAddInvalid()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    check(!repo.add(nullptr), "Tu choi nullptr");

    Member* invalidId = new Member(
        "ABC0001", "Nguyen Van A", "0901234567", 0
    );

    Member* invalidName = new Member(
        "MEM0002", "   ", "0912345678", 0
    );

    Member* invalidPhone = new Member(
        "MEM0003", "Tran Van B", "123", 0
    );

    Member* invalidPoints = new Member(
        "MEM0004", "Le Van C", "0987654321", -5
    );

    Member* nonZeroPoints = new Member(
        "MEM0005", "Pham Van D", "0976543210", 100
    );

    check(!repo.add(invalidId), "Tu choi ID sai");
    check(!repo.add(invalidName), "Tu choi ten rong");
    check(!repo.add(invalidPhone), "Tu choi phone sai");
    check(!repo.add(invalidPoints), "Tu choi diem am");
    check(!repo.add(nonZeroPoints), "Tu choi diem ban dau khac 0");
    check(repo.count() == 0, "Khong them du lieu sai");

    delete invalidId;
    delete invalidName;
    delete invalidPhone;
    delete invalidPoints;
    delete nonZeroPoints;
}

// Test 8: Add khi chua load
void testAddBeforeLoad()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);

    Member* member = new Member(
        "MEM0001", "Nguyen Van A", "0901234567", 0
    );

    check(!repo.add(member), "Add khi chua load");
    check(repo.count() == 0, "Count khi chua load");

    delete member;
}

// Test 9: Update thong tin hop le
void testUpdateValid()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|100\n"
    );

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member newValues(
        "MEM0001", "Nguyen Van B", "0987654321", 100
    );

    check(repo.update(newValues), "Update hop le");

    Member* member = repo.findById("MEM0001");

    check(member != nullptr &&
          member->getFullName() == "Nguyen Van B",
          "Update ho ten");

    check(member != nullptr &&
          member->getPhone() == "0987654321",
          "Update phone");

    check(repo.findByPhone("0901234567") == nullptr,
          "Xoa phone cu khoi map");

    check(repo.findByPhone("0987654321") == member,
          "Them phone moi vao map");

    check(member != nullptr && member->getPoints() == 100,
          "Giu nguyen diem");
}

// Test 10: Update giu nguyen phone
void testUpdateSamePhone()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|0\n"
    );

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member values(
        "MEM0001", "Nguyen Van B", "0901234567", 0
    );

    check(repo.update(values), "Update giu nguyen phone");

    check(repo.findByPhone("0901234567") ==
          repo.findById("MEM0001"),
          "Map van dong bo");
}

// Test 11: Update trung phone
void testUpdateDuplicatePhone()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|0\n"
        "MEM0002|Tran Van B|0912345678|0\n"
    );

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member values(
        "MEM0001", "Nguyen Van C", "0912345678", 0
    );

    check(!repo.update(values), "Tu choi update trung phone");

    Member* member = repo.findById("MEM0001");

    check(member != nullptr &&
          member->getPhone() == "0901234567",
          "Giu phone cu khi update loi");

    check(member != nullptr &&
          member->getFullName() == "Nguyen Van A",
          "Giu ten cu khi update loi");
}

// Test 12: Khong cho update diem
void testUpdatePoints()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|100\n"
    );

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member values(
        "MEM0001", "Nguyen Van A", "0901234567", 200
    );

    check(!repo.update(values), "Tu choi thay doi diem");

    Member* member = repo.findById("MEM0001");

    check(member != nullptr && member->getPoints() == 100,
          "Diem khong thay doi");
}

// Test 13: Update ID khong ton tai
void testUpdateNotFound()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);
    repo.load();

    Member values(
        "MEM0001", "Nguyen Van A", "0901234567", 0
    );

    check(!repo.update(values), "Update ID khong ton tai");
}

// Test 14: Save va load lai
void testSaveReload()
{
    writeTestFile("");

    {
        MemberRepository repo(TEST_FILE);
        repo.load();

        Member* member = new Member(
            "MEM0001", "Nguyen Van A", "0901234567", 0
        );

        bool added = repo.add(member);
        check(added, "Add truoc save");

        if (!added) delete member;

        check(repo.save(), "Save thanh cong");
    }

    MemberRepository repo2(TEST_FILE);
    repo2.load();

    Member* restored = repo2.findById("MEM0001");

    check(repo2.count() == 1, "Count sau reload");

    check(restored != nullptr &&
          restored->getPhone() == "0901234567",
          "Du lieu giu nguyen sau reload");

    check(readTestFile().find(
        "MEM0001|Nguyen Van A|0901234567|0"
    ) != string::npos, "Dinh dang TXT dung");
}

// Test 15: Save khi chua load
void testSaveBeforeLoad()
{
    writeTestFile("");

    MemberRepository repo(TEST_FILE);

    bool thrown = false;

    try
    {
        repo.save();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Save khi chua load nem ngoai le");
}

// Test 16: Load trung ID
void testLoadDuplicateId()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|0\n"
        "MEM0001|Tran Van B|0912345678|0\n"
    );

    MemberRepository repo(TEST_FILE);
    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load trung ID nem ngoai le");
    check(!repo.isLoaded(), "Trang thai sau loi trung ID");
}

// Test 17: Load trung phone
void testLoadDuplicatePhone()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|0\n"
        "MEM0002|Tran Van B|0901234567|0\n"
    );

    MemberRepository repo(TEST_FILE);
    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load trung phone nem ngoai le");
    check(repo.count() == 0, "Don du lieu khi trung phone");
    check(!repo.isLoaded(), "Reset loaded khi trung phone");
    check(repo.findById("MEM0001") == nullptr,
          "Xoa map ID khi load loi");
}

// Test 18: Load sai dinh dang
void testLoadInvalidFormat()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567\n"
    );

    MemberRepository repo(TEST_FILE);
    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load thieu truong nem ngoai le");
    check(!repo.isLoaded(), "Khong loaded khi sai dinh dang");
}

// Test 19: Load diem sai
void testLoadInvalidPoints()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|12abc\n"
    );

    MemberRepository repo(TEST_FILE);
    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load diem khong phai so");
}

// Test 20: Load diem vuot int
void testLoadOverflowPoints()
{
    writeTestFile(
        "MEM0001|Nguyen Van A|0901234567|999999999999999999999\n"
    );

    MemberRepository repo(TEST_FILE);
    bool thrown = false;

    try
    {
        repo.load();
    }
    catch (const runtime_error&)
    {
        thrown = true;
    }

    check(thrown, "Load diem vuot gioi han int");
}

int main()
{
    try
    {
        testLoadEmpty();
        testLoadValid();
        testLoadTwice();
        testAddValid();
        testAddDuplicateId();
        testAddDuplicatePhone();
        testAddInvalid();
        testAddBeforeLoad();
        testUpdateValid();
        testUpdateSamePhone();
        testUpdateDuplicatePhone();
        testUpdatePoints();
        testUpdateNotFound();
        testSaveReload();
        testSaveBeforeLoad();
        testLoadDuplicateId();
        testLoadDuplicatePhone();
        testLoadInvalidFormat();
        testLoadInvalidPoints();
        testLoadOverflowPoints();
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

    remove(TEST_FILE.c_str());

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
