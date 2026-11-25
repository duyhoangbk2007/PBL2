#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <stdexcept>

#include "../src/data_access/IRepository.h"
#include "../src/data_access/MemberRepository.h"
#include "../src/utils/Validator.h"

using namespace std;

int passed = 0;
int failed = 0;

const string FILE_PATH = "test_members_temp.txt";

void check(bool condition, const string& message)
{
    if (condition)
    {
        cout << "[PASS] " << message << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << message << endl;
        failed++;
    }
}

void writeFile(const string& content)
{
    ofstream file(FILE_PATH);

    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }

    file << content;
}

void checkAddFailed(MemberRepository& repo,
                    Member* member,
                    const string& message)
{
    bool result = repo.add(member);
    check(!result, message);

    if (!result)
    {
        delete member;
    }
}

// ==================================================
// 1. KIEM THU VALIDATOR
// ==================================================

void testValidator()
{
    cout << "\n===== VALIDATOR =====\n";

    check(Validator::isValidPhone("0912345678"),
          "Phone hop le");

    check(!Validator::isValidPhone(""),
          "Phone rong");

    check(!Validator::isValidPhone("091234567"),
          "Phone thieu chu so");

    check(!Validator::isValidPhone("09123456789"),
          "Phone thua chu so");

    check(!Validator::isValidPhone("1912345678"),
          "Phone khong bat dau bang 0");

    check(!Validator::isValidPhone("091234567a"),
          "Phone chua chu cai");

    check(!Validator::isValidPhone("09123 5678"),
          "Phone chua khoang trang");

    check(!Validator::isValidPhone("09123-5678"),
          "Phone chua ky tu dac biet");
}

// ==================================================
// 2. KIEM THU TRUOC KHI LOAD
// ==================================================

void testBeforeLoad()
{
    cout << "\n===== BEFORE LOAD =====\n";

    MemberRepository repo(FILE_PATH);

    check(!repo.isLoaded(),
          "Trang thai ban dau");

    check(repo.count() == 0,
          "Danh sach ban dau rong");

    check(repo.findById("MEM0001") == nullptr,
          "Find ID truoc load");

    check(repo.findByPhone("0912345678") == nullptr,
          "Find phone truoc load");

    check(repo.getAt(0) == nullptr,
          "GetAt truoc load");

    check(!repo.save(),
          "Save truoc load");

    checkAddFailed(
        repo,
        new Member("MEM0001", "Nguyen Van A", "0912345678", 0),
        "Add truoc load"
    );

    Member newValues("MEM0001", "Nguyen Van B", "0912345678", 0);

    check(!repo.update(newValues),
          "Update truoc load");

    check(!repo.removeById("MEM0001"),
          "Remove truoc load");

    check(repo.findByPhone("0912345678") == nullptr,
          "Map khong thay doi truoc load");
}

// ==================================================
// 3. KIEM THU LOAD
// ==================================================

void testLoad()
{
    cout << "\n===== LOAD =====\n";

    writeFile(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0002|Tran Van B|0987654321|150\n"
    );

    MemberRepository repo(FILE_PATH);

    check(repo.load(),
          "Load file hop le");

    check(repo.isLoaded(),
          "Trang thai sau load");

    check(repo.count() == 2,
          "So luong sau load");

    Member* member = repo.findById("MEM0001");

    check(member != nullptr,
          "Tim Member sau load");

    check(member != nullptr &&
          member->getFullName() == "Nguyen Van A",
          "Ho ten sau load");

    check(member != nullptr &&
          member->getPhone() == "0912345678",
          "Phone sau load");

    Member* second = repo.findById("MEM0002");

    check(second != nullptr &&
          second->getPoints() == 150,
          "Points sau load");

    check(member != nullptr &&
          repo.findByPhone("0912345678") == member,
          "Map tro dung Member 1");

    check(second != nullptr &&
          repo.findByPhone("0987654321") == second,
          "Map tro dung Member 2");

    check(!repo.load(),
          "Khong cho load lai");

    check(repo.count() == 2,
          "Danh sach khong doi khi load lai");

    check(member != nullptr &&
          repo.findByPhone("0912345678") == member,
          "Map khong doi khi load lai");
}

void testLoadEmpty()
{
    cout << "\n===== LOAD EMPTY =====\n";

    writeFile("");

    MemberRepository repo(FILE_PATH);

    check(repo.load(),
          "Load file rong");

    check(repo.isLoaded(),
          "Loaded file rong");

    check(repo.count() == 0,
          "Count file rong");

    check(repo.findByPhone("0912345678") == nullptr,
          "Map file rong");
}

void testLoadMissingFile()
{
    cout << "\n===== LOAD MISSING FILE =====\n";

    remove(FILE_PATH.c_str());

    MemberRepository repo(FILE_PATH);

    bool rejected = false;

    try
    {
        rejected = !repo.load();
    }
    catch (const exception&)
    {
        rejected = true;
    }

    check(rejected,
          "Load file khong ton tai");

    check(!repo.isLoaded(),
          "Khong loaded khi thieu file");

    check(repo.count() == 0,
          "Danh sach rong khi thieu file");
}

// ==================================================
// 4. KIEM THU LOAD DU LIEU LOI
// ==================================================

void checkInvalidLoad(const string& content,
                      const string& message)
{
    writeFile(content);

    MemberRepository repo(FILE_PATH);

    bool hasException = false;

    try
    {
        repo.load();
    }
    catch (const exception&)
    {
        hasException = true;
    }

    check(hasException, message);

    check(!repo.isLoaded(),
          message + " - loaded rollback");

    check(repo.count() == 0,
          message + " - danh sach rollback");

    check(repo.findByPhone("0912345678") == nullptr,
          message + " - map rollback");
}

void testInvalidLoad()
{
    cout << "\n===== INVALID LOAD =====\n";

    checkInvalidLoad(
        "MEM0001|Nguyen Van A|0912345678\n",
        "Thieu truong"
    );

    checkInvalidLoad(
        "MEM0001|Nguyen Van A|0912345678|0|extra\n",
        "Thua truong"
    );

    checkInvalidLoad(
        "MOV0001|Nguyen Van A|0912345678|0\n",
        "Sai tien to ID"
    );

    checkInvalidLoad(
        "MEM0000|Nguyen Van A|0912345678|0\n",
        "ID khong hop le"
    );

    checkInvalidLoad(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0001|Tran Van B|0987654321|0\n",
        "Trung ID"
    );

    checkInvalidLoad(
        "MEM0001||0912345678|0\n",
        "Ho ten rong"
    );

    checkInvalidLoad(
        "MEM0001|Nguyen Van A|1912345678|0\n",
        "Phone khong hop le"
    );

    checkInvalidLoad(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0002|Tran Van B|0912345678|0\n",
        "Trung phone"
    );

    vector<string> invalidPoints =
    {
        "",
        "-1",
        "+10",
        "10.5",
        "10abc",
        " 10",
        "10 ",
        "999999999999999999999"
    };

    for (const string& points : invalidPoints)
    {
        checkInvalidLoad(
            "MEM0001|Nguyen Van A|0912345678|" + points + "\n",
            "Points khong hop le: [" + points + "]"
        );
    }
}

// ==================================================
// 5. KIEM THU LOAD LAI SAU ROLLBACK
// ==================================================

void testReloadAfterFailure()
{
    cout << "\n===== RELOAD AFTER FAILURE =====\n";

    writeFile(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0002|Tran Van B|0987654321|0\n"
        "MEM0003|Le Van C|0912345678|0\n"
    );

    MemberRepository repo(FILE_PATH);

    bool hasException = false;

    try
    {
        repo.load();
    }
    catch (const exception&)
    {
        hasException = true;
    }

    check(hasException,
          "Load that bai do trung phone");

    check(!repo.isLoaded(),
          "Rollback loaded");

    check(repo.count() == 0,
          "Rollback danh sach");

    check(repo.findByPhone("0912345678") == nullptr,
          "Rollback phone 1");

    check(repo.findByPhone("0987654321") == nullptr,
          "Rollback phone 2");

    writeFile(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0002|Tran Van B|0987654321|100\n"
    );

    check(repo.load(),
          "Load lai sau khi sua file");

    check(repo.isLoaded(),
          "Loaded sau rollback");

    check(repo.count() == 2,
          "Count sau khi load lai");

    Member* member = repo.findById("MEM0001");

    check(member != nullptr &&
          repo.findByPhone("0912345678") == member,
          "Map dung sau khi load lai");
}

// ==================================================
// 6. KIEM THU ADD
// ==================================================

void testAdd()
{
    cout << "\n===== ADD =====\n";

    writeFile("");

    MemberRepository repo(FILE_PATH);

    check(repo.load(), "Load truoc add");

    Member* member = new Member(
        "MEM0001",
        "Nguyen Van A",
        "0912345678",
        0
    );

    bool added = repo.add(member);

    check(added,
          "Add Member hop le");

    if (!added)
    {
        delete member;
        return;
    }

    check(repo.count() == 1,
          "Count sau add");

    check(repo.findById("MEM0001") == member,
          "Find ID sau add");

    check(repo.findByPhone("0912345678") == member,
          "Map sau add");

    check(!repo.add(nullptr),
          "Add nullptr");

    checkAddFailed(
        repo,
        new Member("MOV0002", "Tran Van B", "0987654321", 0),
        "Add sai ID"
    );

    checkAddFailed(
        repo,
        new Member("MEM0002", "", "0987654321", 0),
        "Add ho ten rong"
    );

    checkAddFailed(
        repo,
        new Member("MEM0002", "Tran Van B", "123", 0),
        "Add phone khong hop le"
    );

    checkAddFailed(
        repo,
        new Member("MEM0002", "Tran Van B", "0987654321", -1),
        "Add points am"
    );

    checkAddFailed(
        repo,
        new Member("MEM0002", "Tran Van B", "0987654321", 100),
        "Add points khac 0"
    );

    checkAddFailed(
        repo,
        new Member("MEM0002", "Tran Van B", "0912345678", 0),
        "Add trung phone"
    );

    // Phone hop le nhung ID trung.
    // Kiem tra rollback map khi repo.add() tra false.
    checkAddFailed(
        repo,
        new Member("MEM0001", "Tran Van B", "0987654321", 0),
        "Add trung ID"
    );

    check(repo.findByPhone("0987654321") == nullptr,
          "Rollback map khi trung ID");

    check(!repo.add(member),
          "Add lai con tro da quan ly");

    check(repo.count() == 1,
          "Count khong doi sau add loi");

    check(repo.findById("MEM0001") == member,
          "Member cu khong thay doi");

    check(repo.findByPhone("0912345678") == member,
          "Map cu khong thay doi");

    Member* second = new Member(
        "MEM0002",
        "Tran Van B",
        "0987654321",
        0
    );

    bool addedSecond = repo.add(second);

    check(addedSecond,
          "Add lai phone sau rollback");

    if (!addedSecond)
    {
        delete second;
        return;
    }

    check(repo.count() == 2,
          "Count sau add Member 2");

    check(repo.findByPhone("0987654321") == second,
          "Map Member 2 hop le");
}

// ==================================================
// 7. KIEM THU UPDATE
// ==================================================

void testUpdate()
{
    cout << "\n===== UPDATE =====\n";

    writeFile(
        "MEM0001|Nguyen Van A|0912345678|100\n"
        "MEM0002|Tran Van B|0987654321|0\n"
    );

    MemberRepository repo(FILE_PATH);

    check(repo.load(), "Load truoc update");

    Member* member = repo.findById("MEM0001");
    Member* second = repo.findById("MEM0002");

    if (member == nullptr || second == nullptr)
    {
        check(false, "Du lieu update khong ton tai");
        return;
    }

    Member samePhone(
        "MEM0001",
        "Nguyen Van A Updated",
        "0912345678",
        100
    );

    check(repo.update(samePhone),
          "Update ho ten");

    check(member->getFullName() == "Nguyen Van A Updated",
          "Ho ten da thay doi");

    check(repo.findByPhone("0912345678") == member,
          "Map giu nguyen khi update ho ten");

    Member newPhone(
        "MEM0001",
        "Nguyen Van A New",
        "0901234567",
        100
    );

    check(repo.update(newPhone),
          "Update phone");

    check(member->getPhone() == "0901234567",
          "Phone da thay doi");

    check(member->getFullName() == "Nguyen Van A New",
          "Ho ten moi");

    check(repo.findByPhone("0912345678") == nullptr,
          "Phone cu da bi xoa khoi map");

    check(repo.findByPhone("0901234567") == member,
          "Phone moi tro dung Member");

    check(repo.findById("MEM0001") == member,
          "Dia chi Member khong thay doi");

    check(member->getMemberId() == "MEM0001",
          "ID khong thay doi");

    check(member->getPoints() == 100,
          "Points khong thay doi");

    Member duplicatePhone(
        "MEM0001",
        "Ten khong duoc cap nhat",
        "0987654321",
        100
    );

    check(!repo.update(duplicatePhone),
          "Update trung phone Member khac");

    Member invalidPoints(
        "MEM0001",
        "Nguyen Van A",
        "0901234567",
        200
    );

    check(!repo.update(invalidPoints),
          "Update thay doi points");

    Member invalidPhone(
        "MEM0001",
        "Nguyen Van A",
        "123",
        100
    );

    check(!repo.update(invalidPhone),
          "Update phone khong hop le");

    Member invalidName(
        "MEM0001",
        "",
        "0901234567",
        100
    );

    check(!repo.update(invalidName),
          "Update ho ten rong");

    Member missingMember(
        "MEM9999",
        "Le Van C",
        "0934567890",
        0
    );

    check(!repo.update(missingMember),
          "Update Member khong ton tai");

    check(member->getFullName() == "Nguyen Van A New",
          "Ho ten khong doi sau update loi");

    check(member->getPhone() == "0901234567",
          "Phone khong doi sau update loi");

    check(member->getPoints() == 100,
          "Points khong doi sau update loi");

    check(repo.findByPhone("0901234567") == member,
          "Map Member 1 khong doi");

    check(repo.findByPhone("0987654321") == second,
          "Map Member 2 khong doi");

    check(repo.count() == 2,
          "Count khong doi sau update");
}

// ==================================================
// 8. KIEM THU REMOVE
// ==================================================

void testRemove()
{
    cout << "\n===== REMOVE =====\n";

    writeFile("MEM0001|Nguyen Van A|0912345678|100\n");

    MemberRepository repo(FILE_PATH);

    check(repo.load(), "Load truoc remove");

    Member* member = repo.findById("MEM0001");

    check(!repo.removeById("MEM0001"),
          "Khong cho xoa Member ton tai");

    check(!repo.removeById("MEM9999"),
          "Remove Member khong ton tai");

    check(!repo.removeById(""),
          "Remove ID rong");

    check(repo.count() == 1,
          "Count khong doi sau remove");

    check(member != nullptr &&
          repo.findById("MEM0001") == member,
          "Member khong bi xoa");

    check(member != nullptr &&
          repo.findByPhone("0912345678") == member,
          "Map khong doi sau remove");
}

// ==================================================
// 9. KIEM THU TRUY VAN
// ==================================================

void testQuery()
{
    cout << "\n===== QUERY =====\n";

    writeFile(
        "MEM0001|Nguyen Van A|0912345678|0\n"
        "MEM0002|Tran Van B|0987654321|100\n"
    );

    MemberRepository repo(FILE_PATH);

    check(repo.load(), "Load truoc query");

    Member* first = repo.findById("MEM0001");
    Member* second = repo.findById("MEM0002");

    check(first != nullptr,
          "Find ID ton tai");

    check(repo.findById("MEM9999") == nullptr,
          "Find ID khong ton tai");

    check(first != nullptr &&
          repo.findByPhone("0912345678") == first,
          "Find phone ton tai");

    check(repo.findByPhone("0900000000") == nullptr,
          "Find phone khong ton tai");

    check(repo.findByPhone("") == nullptr,
          "Find phone rong");

    check(first != nullptr && repo.getAt(0) == first,
          "GetAt 0");

    check(second != nullptr && repo.getAt(1) == second,
          "GetAt 1");

    check(repo.getAt(-1) == nullptr,
          "GetAt am");

    check(repo.getAt(2) == nullptr,
          "GetAt vuot gioi han");

    check(repo.count() == 2,
          "Count");

    check(repo.isLoaded(),
          "IsLoaded");
}

// ==================================================
// 10. KIEM THU SAVE VA RELOAD
// ==================================================

void testSave()
{
    cout << "\n===== SAVE AND RELOAD =====\n";

    writeFile("");

    {
        MemberRepository repo(FILE_PATH);

        check(repo.load(), "Load truoc save");

        Member* member = new Member(
            "MEM0001",
            "Nguyen Van A",
            "0912345678",
            0
        );

        bool added = repo.add(member);

        check(added, "Add truoc save");

        if (!added)
        {
            delete member;
            return;
        }

        Member newValues(
            "MEM0001",
            "Nguyen Van A Updated",
            "0901234567",
            0
        );

        check(repo.update(newValues),
              "Update truoc save");

        check(repo.save(),
              "Save thanh cong");
    }

    {
        ifstream file(FILE_PATH);
        string line;

        getline(file, line);

        check(line == "MEM0001|Nguyen Van A Updated|0901234567|0",
              "Dung dinh dang TXT");
    }

    {
        MemberRepository repo(FILE_PATH);

        check(repo.load(),
              "Reload sau save");

        check(repo.count() == 1,
              "Count sau reload");

        Member* member = repo.findById("MEM0001");

        check(member != nullptr,
              "Find ID sau reload");

        check(member != nullptr &&
              member->getFullName() == "Nguyen Van A Updated",
              "Ho ten sau reload");

        check(member != nullptr &&
              member->getPhone() == "0901234567",
              "Phone sau reload");

        check(member != nullptr &&
              member->getPoints() == 0,
              "Points sau reload");

        check(member != nullptr &&
              repo.findByPhone("0901234567") == member,
              "Map sau reload");

        check(repo.findByPhone("0912345678") == nullptr,
              "Phone cu khong ton tai sau reload");
    }
}

void testSaveEmpty()
{
    cout << "\n===== SAVE EMPTY =====\n";

    writeFile("");

    MemberRepository repo(FILE_PATH);

    check(repo.load(),
          "Load danh sach rong");

    check(repo.save(),
          "Save danh sach rong");

    ifstream file(FILE_PATH);
    string line;

    check(file.is_open() && !getline(file, line),
          "File sau save rong");
}

// ==================================================
// 11. KIEM THU IREPOSITORY
// ==================================================

void testInterface()
{
    cout << "\n===== IREPOSITORY =====\n";

    writeFile("");

    MemberRepository memberRepo(FILE_PATH);

    IRepository<Member>& repo = memberRepo;
    IRepository<Member>* ptr = &memberRepo;

    check(!repo.isLoaded(),
          "Interface isLoaded truoc load");

    check(repo.load(),
          "Interface load");

    check(ptr->isLoaded(),
          "Interface isLoaded sau load");

    Member* member = new Member(
        "MEM0001",
        "Nguyen Van A",
        "0912345678",
        0
    );

    bool added = repo.add(member);

    check(added,
          "Interface add");

    if (!added)
    {
        delete member;
        return;
    }

    check(ptr->count() == 1,
          "Interface count");

    check(repo.findById("MEM0001") == member,
          "Interface findById");

    check(ptr->getAt(0) == member,
          "Interface getAt");

    Member newValues(
        "MEM0001",
        "Nguyen Van B",
        "0987654321",
        0
    );

    check(repo.update(newValues),
          "Interface update");

    check(member->getFullName() == "Nguyen Van B",
          "Interface update du lieu");

    check(memberRepo.findByPhone("0987654321") == member,
          "Interface update dong bo map");

    check(!ptr->removeById("MEM0001"),
          "Interface removeById");

    check(repo.save(),
          "Interface save");

    check(repo.count() == 1,
          "Interface count sau remove");
}

void testVirtualDestructor()
{
    cout << "\n===== VIRTUAL DESTRUCTOR =====\n";

    writeFile("MEM0001|Nguyen Van A|0912345678|0\n");

    IRepository<Member>* repo =
        new MemberRepository(FILE_PATH);

    check(repo->load(),
          "Load qua con tro interface");

    delete repo;

    check(true,
          "Delete qua con tro interface");
}

// ==================================================
// MAIN
// ==================================================

int main()
{
    try
    {
        testValidator();
        testBeforeLoad();

        testLoad();
        testLoadEmpty();
        testLoadMissingFile();
        testInvalidLoad();
        testReloadAfterFailure();

        testAdd();
        testUpdate();
        testRemove();
        testQuery();

        testSave();
        testSaveEmpty();

        testInterface();
        testVirtualDestructor();
    }
    catch (const exception& e)
    {
        check(false, string("Unexpected exception: ") + e.what());
    }
    catch (...)
    {
        check(false, "Unknown exception");
    }

    remove(FILE_PATH.c_str());

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}