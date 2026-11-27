#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "../src/business/RoomService.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/models/Seat.h"
#include "../src/models/Showtime.h"

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
    if (!out || !(out << data)) throw runtime_error("Khong ghi duoc file test");
}

string readFile(const fs::path& path)
{
    ifstream in(path, ios::binary);
    if (!in) throw runtime_error("Khong doc duoc file test");
    string normalized;
    for (char ch; in.get(ch);)
    {
        if (ch != '\r') normalized += ch;
    }
    return normalized;
}

struct TestFiles
{
    fs::path dir;
    TestFiles()
    {
        static int sequence = 0;
        dir = fs::temp_directory_path() /
              ("pbl2_room_service_" +
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
    "USR0001|admin|secret|Admin User|ADMIN|1\n"
    "USR0002|staff|secret|Staff User|STAFF|1\n";
const string ALL_PERMISSIONS =
    "ADMIN|ROOM_VIEW\nADMIN|ROOM_ADD\nADMIN|ROOM_UPDATE\nADMIN|ROOM_DELETE\n"
    "STAFF|ROOM_VIEW\nSTAFF|ROOM_ADD\nSTAFF|ROOM_UPDATE\nSTAFF|ROOM_DELETE\n";

struct Fixture
{
    TestFiles files;
    RoomRepository concrete;
    IRepository<Room>& repo;
    UserRepository users;
    RolePermissionRepository permissions;
    AuthService auth;
    RoomService service;

    Fixture(const string& rooms = "", const string& rights = ALL_PERMISSIONS)
        : concrete(files.path("rooms.txt").string()), repo(concrete),
          users(files.path("users.txt").string()),
          permissions(files.path("permissions.txt").string()),
          auth(users, permissions), service(repo, auth)
    {
        writeFile(files.path("rooms.txt"), rooms);
        writeFile(files.path("users.txt"), USERS);
        writeFile(files.path("permissions.txt"), rights);
        if (!users.load() || !permissions.load())
            throw runtime_error("Khong nap duoc dependency test");
    }

    void login(const string& name = "admin")
    {
        if (!auth.login(name, "secret"))
            throw runtime_error("Khong dang nhap duoc tai khoan test");
    }
};

struct FailingRepository : IRepository<Room>
{
    Room stored{"ROM0001", "Phong", 5, 6, true};
    bool throwOnAdd = false;
    int updateCalls = 0;

    bool load() override { return true; }
    bool save() override { return true; }
    bool add(Room*) override
    {
        if (throwOnAdd) throw runtime_error("Loi add gia lap");
        return false;
    }
    bool update(const Room&) override
    {
        updateCalls++;
        return false;
    }
    bool removeById(const string&) override { return false; }
    Room* findById(const string& id) override
    {
        return id == stored.getRoomId() ? &stored : nullptr;
    }
    Room* getAt(int index) override { return index == 0 ? &stored : nullptr; }
    int count() override { return 1; }
    bool isLoaded() override { return true; }
};

Room makeRoom(const string& id, const string& name = "Phong moi",
              int rows = 5, int columns = 6, bool active = true)
{
    return Room(id, name, rows, columns, active);
}

string row(const string& id, const string& name, int rows = 5,
           int columns = 6, bool active = true)
{
    return id + "|" + name + "|" + to_string(rows) + "|" +
           to_string(columns) + "|" + (active ? "1" : "0") + "\n";
}

void testInterfaceLoadSaveAndQuery()
{
    cout << "\n===== TEST INTERFACE / LOAD / QUERY =====" << endl;
    check(is_base_of<IRepository<Room>, RoomRepository>::value &&
          is_constructible<RoomService, IRepository<Room>&, AuthService&>::value,
          "Constructor DI qua IRepository<Room>&");
    Fixture f;
    f.login();
    check(!f.repo.isLoaded() && f.service.getRoomCount() == 0 &&
          f.service.findRoomById("ROM0001") == nullptr &&
          f.service.getRoomAt(0) == nullptr, "Query truoc load");
    check(!f.service.save(), "Save truoc load");
    check(f.service.load() && f.repo.isLoaded() && !f.service.load(),
          "Load file rong, load lap");
    check(f.service.save() && readFile(f.files.path("rooms.txt")).empty(),
          "Save file rong");
    Room input = makeRoom("ORIGINAL");
    check(f.service.addRoom(&input) && f.service.save() &&
          readFile(f.files.path("rooms.txt")) == row("ROM0001", "Phong moi"),
          "Save schema Room");
    const Room* found = f.service.findRoomById("ROM0001");
    check(found && found == f.service.getRoomAt(0) &&
          f.service.getRoomCount() == 1 &&
          f.service.getRoomAt(-1) == nullptr &&
          f.service.getRoomAt(1) == nullptr &&
          f.service.findRoomById("ROM9999") == nullptr,
          "Find, getAt, count va bien index");
    Fixture reloaded(readFile(f.files.path("rooms.txt")));
    reloaded.login();
    check(reloaded.service.load() && reloaded.service.getRoomCount() == 1 &&
          reloaded.service.findRoomById("ROM0001") != nullptr,
          "Save roi reload bang file rieng");

    Fixture missing;
    fs::remove(missing.files.path("rooms.txt"));
    check(!missing.service.load() && !missing.repo.isLoaded(), "Load file thieu");
    Fixture invalid("ROM0001|bad\n");
    check(throwsError([&] { invalid.service.load(); }) && !invalid.repo.isLoaded(),
          "Load sai so truong truyen exception");
    Fixture invalidBool("ROM0001|Phong|5|6|2\n");
    check(throwsError([&] { invalidBool.service.load(); }),
          "Load boolean sai truyen exception");
}

void testPermissionOnly()
{
    cout << "\n===== TEST PERMISSION-ONLY =====" << endl;
    Fixture f("", "STAFF|ROOM_VIEW\nSTAFF|ROOM_ADD\nSTAFF|ROOM_UPDATE\nSTAFF|ROOM_DELETE\n");
    check(f.service.load(), "Load khong can Permission");
    Room input = makeRoom("IGNORED");
    check(throwsError([&] { f.service.addRoom(&input); }) &&
          throwsError([&] { f.service.getRoomCount(); }),
          "Chua dang nhap bi tu choi");
    f.login("admin");
    check(throwsError([&] { f.service.addRoom(&input); }) &&
          throwsError([&] { f.service.getRoomCount(); }) && f.repo.count() == 0,
          "ADMIN khong duoc cap quyen bi tu choi");
    f.login("staff");
    check(f.service.addRoom(&input) && f.service.getRoomCount() == 1 &&
          f.service.findRoomById("ROM0001") != nullptr,
          "STAFF duoc cap quyen duoc thao tac");
    User* staff = f.users.findById("USR0002");
    staff->setActive(false);
    check(throwsError([&] { f.service.getRoomCount(); }),
          "Tai khoan inactive bi tu choi");

    Fixture noRights(row("ROM0001", "Phong"), "");
    noRights.service.load();
    noRights.login();
    Room changed = makeRoom("ROM0001", "Ten moi");
    check(throwsError([&] { noRights.service.addRoom(&input); }) &&
          throwsError([&] { noRights.service.updateRoom(changed); }) &&
          throwsError([&] { noRights.service.setRoomActive("ROM0001", false); }) &&
          throwsError([&] { noRights.service.removeRoom("ROM0001"); }) &&
          noRights.repo.count() == 1, "Thieu quyen ghi: khong doi du lieu");
    check(throwsError([&] { noRights.service.findRoomById("ROM0001"); }) &&
          throwsError([&] { noRights.service.getRoomAt(0); }) &&
          throwsError([&] { noRights.service.getRoomCount(); }),
          "Thieu ROOM_VIEW: ba API query bi tu choi");
}

void testAddUpdateAndOwnership()
{
    cout << "\n===== TEST ADD / UPDATE / OWNERSHIP =====" << endl;
    Fixture f;
    f.login();
    Room input = makeRoom("ORIGINAL");
    check(!f.service.addRoom(nullptr) && !f.service.addRoom(&input),
          "Add nullptr, truoc load");
    f.service.load();
    check(f.service.addRoom(&input) && input.getRoomId() == "ORIGINAL" &&
          f.repo.findById("ROM0001") != &input,
          "Sinh ROM0001, sao chep, caller giu input");
    Room second = makeRoom("ANY", "Phong hai");
    check(f.service.addRoom(&second) && f.repo.findById("ROM0002") != nullptr,
          "Sinh ID ke tiep");
    Room bad = makeRoom("ANY", "", 5, 6);
    int count = f.repo.count();
    check(!f.service.addRoom(&bad) && f.repo.count() == count &&
          bad.getRoomId() == "ANY", "Validation fail: count va input khong doi");
    Room* first = f.repo.findById("ROM0001");
    Room renamed = makeRoom("ROM0001", "Ten moi", 6, 7);
    check(f.service.updateRoom(renamed) && f.repo.findById("ROM0001") == first &&
          first->getRoomId() == "ROM0001" && first->getName() == "Ten moi" &&
          first->getRowCount() == 6 && first->getColumnCount() == 7,
          "Update ten va kich thuoc giu ID, dia chi");
    Room missing = makeRoom("ROM9999");
    Room inactive = makeRoom("ROM0001", "Ten khac", 6, 7, false);
    Room invalid = makeRoom("ROM0001", "", 6, 7);
    check(!f.service.updateRoom(missing) && !f.service.updateRoom(inactive) &&
          !f.service.updateRoom(invalid) && first->getName() == "Ten moi" &&
          first->isActive() && f.repo.count() == count,
          "Update sai khong doi du lieu");
    Fixture exhausted(row("ROM9999", "Cuoi day"));
    exhausted.login();
    exhausted.service.load();
    Room next = makeRoom("ORIGINAL");
    check(throwsError([&] { exhausted.service.addRoom(&next); }) &&
          exhausted.repo.count() == 1 && next.getRoomId() == "ORIGINAL",
          "Het day ID: exception, caller giu input");
}

void testLinksAndActive()
{
    cout << "\n===== TEST LINKS / ACTIVE =====" << endl;
    Fixture f(row("ROM0001", "Phong", 5, 6));
    f.login();
    f.service.load();
    Room* room = f.repo.findById("ROM0001");
    check(room && room->getShowtimeAt(0) == nullptr &&
          room->getShowtimeAt(-1) == nullptr, "getShowtimeAt danh sach rong, index am");
    Seat seat("ROM0001", "A01", 5, 6, SeatType::STANDARD);
    check(room->linkSeat(&seat), "Link Seat vao Room");
    Room small = makeRoom("ROM0001", "Phong", 4, 5);
    check(!f.service.updateRoom(small) && room->getRowCount() == 5 &&
          room->getColumnCount() == 6 && !f.service.removeRoom("ROM0001"),
          "Seat chan thu nho vuot vi tri va xoa phong");
    Showtime future("SHW0001", "MOV0001", "ROM0001", "2099-01-01T10:00:00",
                    "2099-01-01T12:00:00", ShowtimeStatus::SCHEDULED);
    check(room->linkShowtime(&future) && room->getShowtimeAt(0) == &future &&
          room->getShowtimeAt(1) == nullptr, "Link Showtime, index hop le va bang count");
    Room larger = makeRoom("ROM0001", "Phong", 7, 8);
    check(!f.service.updateRoom(larger) && room->getRowCount() == 5 &&
          !f.service.removeRoom("ROM0001"),
          "Showtime chan doi kich thuoc va xoa phong");
    check(!f.service.setRoomActive("ROM0001", false) && room->isActive() &&
          room->getSeatCount() == 1 && room->getShowtimeCount() == 1,
          "Suat tuong lai SCHEDULED chan tat phong, giu lien ket");
    future.setStatus(ShowtimeStatus::CANCELLED);
    check(f.service.setRoomActive("ROM0001", false) && !room->isActive() &&
          room->getName() == "Phong" && room->getSeatCount() == 1 &&
          room->getShowtimeAt(0) == &future,
          "Suat CANCELLED cho tat phong, giu truong va lien ket");
    check(f.service.setRoomActive("ROM0001", false) &&
          f.service.setRoomActive("ROM0001", true) && room->isActive(),
          "Trang thai da dung va bat lai");
    check(room->unlinkShowtime(&future) && room->getShowtimeAt(0) == nullptr &&
          room->unlinkSeat(&seat) && f.service.removeRoom("ROM0001"),
          "Unlink Showtime/Seat roi xoa phong");
}

void testBeforeLoadAndPastShowtime()
{
    cout << "\n===== TEST BEFORE LOAD / PAST SHOWTIME =====" << endl;
    Fixture f;
    f.login();
    Room values = makeRoom("ROM0001");
    check(!f.service.updateRoom(values) &&
          !f.service.setRoomActive("ROM0001", false) &&
          !f.service.removeRoom("ROM0001"),
          "Update, setActive, remove truoc load");
    f.service.load();
    check(!f.service.setRoomActive("ROM9999", false) &&
          !f.service.removeRoom("ROM9999"), "ID khong ton tai");
    check(f.service.addRoom(&values) &&
          f.service.setRoomActive("ROM0001", false) &&
          f.service.setRoomActive("ROM0001", true),
          "Tat va bat phong khong co Showtime");
    Room* room = f.repo.findById("ROM0001");
    Showtime past("SHW0002", "MOV0001", "ROM0001", "2020-01-01T10:00:00",
                  "2020-01-01T12:00:00", ShowtimeStatus::SCHEDULED);
    check(room->linkShowtime(&past) &&
          f.service.setRoomActive("ROM0001", false) && !room->isActive(),
          "Suat SCHEDULED qua khu khong chan tat phong");
    check(room->unlinkShowtime(&past) && room->getShowtimeAt(0) == nullptr,
          "getShowtimeAt sau unlink");
}

void testRepositoryFailures()
{
    cout << "\n===== TEST REPOSITORY FAILURES =====" << endl;
    Fixture fixture;
    fixture.login();
    FailingRepository fake;
    IRepository<Room>& interface = fake;
    RoomService service(interface, fixture.auth);
    Room input = makeRoom("ORIGINAL");
    check(!service.addRoom(&input) && fake.count() == 1 &&
          input.getRoomId() == "ORIGINAL",
          "Repository add false: caller giu input, du lieu khong doi");
    fake.throwOnAdd = true;
    check(throwsError([&] { service.addRoom(&input); }) &&
          fake.count() == 1 && input.getRoomId() == "ORIGINAL",
          "Repository add nem exception: Service truyen lai, input con song");
    check(!service.setRoomActive("ROM0001", false) &&
          fake.stored.isActive() && fake.updateCalls == 1,
          "setRoomActive truyen false tu Repository.update");
    check(service.setRoomActive("ROM0001", true) && fake.updateCalls == 1,
          "Trang thai da dung khong goi update");
}

int main()
{
    try
    {
        testInterfaceLoadSaveAndQuery();
        testPermissionOnly();
        testAddUpdateAndOwnership();
        testLinksAndActive();
        testBeforeLoadAndPastShowtime();
        testRepositoryFailures();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
