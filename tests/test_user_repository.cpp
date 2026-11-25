#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "../src/data_access/IRepository.h"
#include "../src/data_access/UserRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_user_repository_users.txt";
const string INVALID_FILE = "test_user_repository_invalid.txt";
const string MISSING_FILE = "test_user_repository_missing.txt";

const string VALID_USERS =
    "USR0001|admin|secret1|Quan tri vien|ADMIN|1\n"
    "USR0002|staff|secret2|Nhan vien|STAFF|0\n";

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

void writeTestFile(const string& path, const string& content)
{
    ofstream file(path);
    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }
    file << content;
    if (!file)
    {
        throw runtime_error("Khong ghi duoc file test");
    }
}

User* createUser(const string& id, const string& username,
                 const string& password = "secret",
                 const string& fullName = "Nguoi dung",
                 UserRole role = UserRole::STAFF, bool active = true)
{
    return new User(id, username, password, fullName, role, active);
}

void checkRejectedUpdate(IRepository<User>& repo, const User& values,
                         User* original, const string& name)
{
    const int oldCount = repo.count();
    const string oldId = original->getUserId();
    const string oldUsername = original->getUsername();
    const string oldPassword = original->getPassword();
    const string oldFullName = original->getFullName();
    const UserRole oldRole = original->getRole();
    const bool oldActive = original->isActive();
    const bool rejected = !repo.update(values);
    check(rejected && repo.count() == oldCount &&
          repo.findById(oldId) == original &&
          original->getUserId() == oldId &&
          original->getUsername() == oldUsername &&
          original->getPassword() == oldPassword &&
          original->getFullName() == oldFullName &&
          original->getRole() == oldRole &&
          original->isActive() == oldActive, name);
}

void testInterface()
{
    cout << "\n===== TEST INTERFACE =====" << endl;
    check(is_abstract<IRepository<User>>::value, "IRepository<User> la lop truu tuong");
    check(is_base_of<IRepository<User>, UserRepository>::value,
          "UserRepository ke thua IRepository<User>");
    check(has_virtual_destructor<IRepository<User>>::value,
          "IRepository<User> co destructor ao");
    check(!is_abstract<UserRepository>::value,
          "UserRepository trien khai du interface");
}

void testBeforeLoad(IRepository<User>& repo)
{
    cout << "\n===== TEST BEFORE LOAD =====" << endl;
    check(!repo.isLoaded() && repo.count() == 0, "Chua load va danh sach rong");
    check(repo.findById("USR0001") == nullptr && repo.getAt(0) == nullptr,
          "Truy van truoc load");
    check(!repo.save(), "Khong save truoc load");
    User* user = createUser("USR0003", "newstaff");
    const bool added = repo.add(user);
    check(!added, "Khong add truoc load");
    if (!added)
    {
        delete user;
    }
    User values("USR0001", "changed", "newpass", "Ten moi", UserRole::ADMIN, true);
    check(!repo.update(values), "Khong update truoc load");
    check(!repo.removeById("USR0001"), "Khong remove truoc load");
    check(repo.count() == 0 && !repo.isLoaded(), "Thao tac loi khong doi trang thai");
}

void testLoadAndQuery(IRepository<User>& repo, UserRepository& concrete)
{
    cout << "\n===== TEST LOAD AND QUERY =====" << endl;
    check(repo.load(), "Load file hop le");
    check(repo.isLoaded() && repo.count() == 2, "Trang thai va count sau load");
    check(!repo.load() && repo.count() == 2, "Load lan hai tra false, khong doi du lieu");
    User* first = repo.findById("USR0001");
    User* second = repo.findById("USR0002");
    check(first != nullptr && first->getUsername() == "admin" &&
          first->getPassword() == "secret1" &&
          first->getFullName() == "Quan tri vien" &&
          first->getRole() == UserRole::ADMIN && first->isActive(),
          "Du lieu Admin sau load");
    check(second != nullptr && second->getUsername() == "staff" &&
          second->getRole() == UserRole::STAFF && !second->isActive(),
          "Du lieu Staff sau load");
    check(repo.getAt(0) == first && repo.getAt(1) == second,
          "GetAt theo thu tu file");
    check(repo.getAt(-1) == nullptr && repo.getAt(2) == nullptr,
          "GetAt ngoai pham vi");
    check(repo.findById("USR9999") == nullptr, "Khong tim thay ID thieu");
    check(concrete.findByUsername("admin") == first &&
          concrete.findByUsername("staff") == second &&
          concrete.findByUsername("ADMIN") == nullptr &&
          concrete.findByUsername("missing") == nullptr,
          "Tim username va phan biet hoa thuong");
}

void testAdd(IRepository<User>& repo, UserRepository& concrete)
{
    cout << "\n===== TEST ADD =====" << endl;
    User* user = createUser("USR0003", "third");
    const bool added = repo.add(user);
    check(added, "Add User hop le");
    if (!added)
    {
        delete user;
    }
    check(added && repo.findById("USR0003") == user &&
          concrete.findByUsername("third") == user && repo.count() == 3,
          "Repository giu con tro va tang count");
    check(!repo.add(nullptr), "Tu choi nullptr");
    if (added)
    {
        check(!repo.add(user), "Tu choi con tro trung");
    }

    User* duplicateId = createUser("USR0001", "different");
    check(!repo.add(duplicateId), "Tu choi ID trung");
    delete duplicateId;
    User* duplicateUsername = createUser("USR0004", "staff");
    check(!repo.add(duplicateUsername), "Tu choi username trung");
    delete duplicateUsername;
    User* invalidId = createUser("MOV0004", "invalidid");
    check(!repo.add(invalidId), "Tu choi ID sai tien to");
    delete invalidId;
    User* invalidText = createUser("USR0004", "", "secret");
    check(!repo.add(invalidText), "Tu choi username rong");
    delete invalidText;
    invalidText = createUser("USR0004", " leading");
    check(!repo.add(invalidText), "Tu choi username co dau cach dau");
    delete invalidText;
    invalidText = createUser("USR0004", "trailing ");
    check(!repo.add(invalidText), "Tu choi username co dau cach cuoi");
    delete invalidText;
    invalidText = createUser("USR0004", "valid", "");
    check(!repo.add(invalidText), "Tu choi password rong");
    delete invalidText;
    invalidText = createUser("USR0004", "valid", "secret", "");
    check(!repo.add(invalidText), "Tu choi fullName rong");
    delete invalidText;
    User* invalidRole = createUser("USR0004", "valid", "secret", "Ten",
                                   static_cast<UserRole>(99));
    check(!repo.add(invalidRole), "Tu choi role sai");
    delete invalidRole;
    check(repo.count() == 3 && repo.findById("USR0004") == nullptr,
          "Add loi khong doi danh sach");
}

void testUpdate(IRepository<User>& repo, UserRepository& concrete)
{
    cout << "\n===== TEST UPDATE =====" << endl;
    User* original = repo.findById("USR0001");
    if (original == nullptr)
    {
        check(false, "Co User de update");
        return;
    }
    const int originalCount = repo.count();
    User changed("USR0001", "admin2", "newsecret", "Quan tri moi",
                 UserRole::ADMIN, false);
    check(repo.update(changed), "Update bon thuoc tinh");
    check(original->getUsername() == "admin2" &&
          original->getPassword() == "newsecret" &&
          original->getFullName() == "Quan tri moi" && !original->isActive(),
          "Gia tri sau update");
    check(repo.findById("USR0001") == original &&
          original->getUserId() == "USR0001" && repo.count() == originalCount &&
          concrete.findByUsername("admin2") == original &&
          concrete.findByUsername("admin") == nullptr,
          "Giu ID, dia chi, count va tim username moi");

    User sameName("USR0001", "admin2", "same-name-pass", "Giu username",
                  UserRole::ADMIN, true);
    check(repo.update(sameName) && original->getUsername() == "admin2" &&
          original->getPassword() == "same-name-pass" &&
          original->getFullName() == "Giu username" && original->isActive(),
          "Update giu nguyen username");

    User duplicateUsername("USR0001", "staff", "bad", "Khong doi",
                           UserRole::ADMIN, false);
    checkRejectedUpdate(repo, duplicateUsername, original,
                        "Tu choi username thuoc User khac, khong doi du lieu");
    User changedRole("USR0001", "admin2", "bad", "Khong doi",
                     UserRole::STAFF, false);
    checkRejectedUpdate(repo, changedRole, original,
                        "Tu choi thay doi role, khong doi du lieu");
    User invalid("USR0001", "", "bad", "Khong doi", UserRole::ADMIN, false);
    checkRejectedUpdate(repo, invalid, original,
                        "Tu choi du lieu sai, khong doi du lieu");
    User missing("USR9999", "missing", "bad", "Khong doi",
                 UserRole::ADMIN, false);
    checkRejectedUpdate(repo, missing, original,
                        "Tu choi ID khong ton tai, khong doi User khac");
}

void testRemove(IRepository<User>& repo)
{
    cout << "\n===== TEST REMOVE =====" << endl;
    User* first = repo.findById("USR0001");
    const int oldCount = repo.count();
    check(!repo.removeById("USR0001") && !repo.removeById("USR9999"),
          "Remove luon tra false");
    check(repo.count() == oldCount && repo.findById("USR0001") == first &&
          repo.isLoaded(), "Remove khong doi du lieu");
}

void testSave(IRepository<User>& repo)
{
    cout << "\n===== TEST SAVE =====" << endl;
    check(repo.save(), "Save thanh cong qua interface");
    UserRepository reloaded(TEST_FILE);
    IRepository<User>& loaded = reloaded;
    check(loaded.load() && loaded.count() == 3, "Load lai du lieu da save");
    User* first = loaded.findById("USR0001");
    check(first != nullptr && first->getUsername() == "admin2" &&
          first->getPassword() == "same-name-pass" &&
          first->getFullName() == "Giu username" && first->isActive() &&
          first->getRole() == UserRole::ADMIN,
          "Thuoc tinh update duoc luu");
    User* third = loaded.findById("USR0003");
    check(third != nullptr && third->getUsername() == "third",
          "User add duoc luu");
}

void testInvalidLoad()
{
    cout << "\n===== TEST INVALID LOAD =====" << endl;
    const string invalidLines[] = {
        "USR0001|other|secret|Ten|ADMIN|1\n", // ID trung
        "USR0009|admin|secret|Ten|STAFF|1\n", // username trung
        "USR0009|other|secret|Ten|STAFF\n", // thieu truong
        "USR0009|other|secret|Ten|STAFF|1|extra\n", // thua truong
        "USR0009|other|secret|Ten|MANAGER|1\n", // role sai
        "USR0009|other|secret|Ten|STAFF|2\n", // active sai
        "MOV0009|other|secret|Ten|STAFF|1\n", // ID sai
        "USR0009| other|secret|Ten|STAFF|1\n", // username sai
        "USR0009|other||Ten|STAFF|1\n" // password sai
    };
    for (int i = 0; i < 9; i++)
    {
        writeTestFile(INVALID_FILE,
                      "USR0001|admin|secret|Quan tri|ADMIN|1\n" + invalidLines[i]);
        UserRepository repo(INVALID_FILE);
        bool threw = false;
        try
        {
            repo.load();
        }
        catch (const runtime_error&)
        {
            threw = true;
        }
        check(threw && !repo.isLoaded() && repo.count() == 0 &&
              repo.findById("USR0001") == nullptr,
              "Load loi rollback " + to_string(i + 1));

        writeTestFile(INVALID_FILE, VALID_USERS);
        bool retried = false;
        try
        {
            retried = repo.load();
        }
        catch (const exception&)
        {
            retried = false;
        }
        check(retried && repo.isLoaded() && repo.count() == 2 &&
              repo.findByUsername("admin") != nullptr,
              "Sua file va load lai " + to_string(i + 1));
    }
    remove(MISSING_FILE.c_str());
    UserRepository missing(MISSING_FILE);
    check(!missing.load() && !missing.isLoaded() && missing.count() == 0,
          "File thieu tra false va khong load");
    writeTestFile(INVALID_FILE, "");
    UserRepository empty(INVALID_FILE);
    check(empty.load() && empty.isLoaded() && empty.count() == 0 &&
          empty.findByUsername("admin") == nullptr,
          "File rong load hop le");
}

void testPolymorphism()
{
    cout << "\n===== TEST POLYMORPHISM =====" << endl;
    IRepository<User>* repo = new UserRepository(TEST_FILE);
    check(repo->load() && repo->isLoaded() && repo->count() == 3 &&
          repo->findById("USR0001") != nullptr,
          "Load va query qua con tro interface");
    delete repo;
}

int main()
{
    try
    {
        writeTestFile(TEST_FILE, VALID_USERS);
        testInterface();
        {
            UserRepository concrete(TEST_FILE);
            IRepository<User>& repo = concrete;
            testBeforeLoad(repo);
            testLoadAndQuery(repo, concrete);
            testAdd(repo, concrete);
            testUpdate(repo, concrete);
            testRemove(repo);
            testSave(repo);
        }
        testPolymorphism();
        testInvalidLoad();
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }
    remove(TEST_FILE.c_str());
    remove(INVALID_FILE.c_str());
    remove(MISSING_FILE.c_str());
    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
