#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <type_traits>

#include "../src/data_access/IRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/models/Seat.h"
#include "../src/models/Showtime.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_room_repository_rooms.txt";
const string INVALID_FILE = "test_room_repository_invalid.txt";
const string MISSING_FILE = "test_room_repository_missing.txt";

// Kiem tra ket qua test
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

// Tao file du lieu test
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

Room* createRoom(const string& id, const string& name,
                 int rows = 8, int columns = 12, bool active = true)
{
    return new Room(id, name, rows, columns, active);
}

void testInterface()
{
    cout << "\n===== TEST INTERFACE =====" << endl;
    check(is_abstract<IRepository<Room>>::value, "IRepository la lop truu tuong");
    check(is_base_of<IRepository<Room>, RoomRepository>::value,
          "RoomRepository ke thua IRepository");
    check(has_virtual_destructor<IRepository<Room>>::value,
          "IRepository co destructor ao");
    check(!is_abstract<RoomRepository>::value,
          "RoomRepository trien khai day du interface");
}

void testBeforeLoad(IRepository<Room>& repo)
{
    cout << "\n===== TEST BEFORE LOAD =====" << endl;
    check(!repo.isLoaded(), "Chua load");
    check(repo.count() == 0, "Danh sach ban dau rong");
    check(repo.findById("ROM0001") == nullptr, "Find truoc load");
    check(repo.getAt(0) == nullptr, "GetAt truoc load");
    check(!repo.save(), "Khong save truoc load");
    Room* room = createRoom("ROM0003", "Phong 3");
    bool added = repo.add(room);
    check(!added, "Khong add truoc load");
    if (!added)
    {
        delete room;
    }
    Room values("ROM0001", "Ten moi", 8, 12, true);
    check(!repo.update(values), "Khong update truoc load");
    check(!repo.removeById("ROM0001"), "Khong remove truoc load");
}

void testLoadAndQuery(IRepository<Room>& repo)
{
    cout << "\n===== TEST LOAD AND QUERY =====" << endl;
    check(repo.load(), "Load file hop le");
    check(repo.isLoaded(), "Loaded sau load");
    check(repo.count() == 2, "Count sau load");
    check(!repo.load(), "Khong load hai lan");
    check(repo.count() == 2, "Load lai khong doi du lieu");
    Room* first = repo.findById("ROM0001");
    Room* second = repo.findById("ROM0002");
    check(first != nullptr && first->getName() == "Phong 1" &&
          first->getRowCount() == 8 && first->getColumnCount() == 12 &&
          first->isActive(), "Du lieu phong thu nhat");
    check(second != nullptr && second->getName() == "Phong 2" &&
          !second->isActive(), "Du lieu phong thu hai");
    check(repo.findById("ROM9999") == nullptr, "Find ID khong ton tai");
    check(repo.getAt(0) == first && repo.getAt(1) == second,
          "GetAt dung thu tu");
    check(repo.getAt(-1) == nullptr && repo.getAt(2) == nullptr,
          "GetAt ngoai pham vi");
}

void testAdd(IRepository<Room>& repo)
{
    cout << "\n===== TEST ADD =====" << endl;
    Room* room = createRoom("ROM0003", "Phong 3");
    bool added = repo.add(room);
    check(added, "Add phong hop le");
    if (!added)
    {
        delete room;
    }
    check(added && repo.findById("ROM0003") == room && repo.count() == 3,
          "Repository giu con tro va tang count");
    check(!repo.add(nullptr), "Tu choi nullptr");
    Room* duplicate = createRoom("ROM0001", "Trung ID");
    added = repo.add(duplicate);
    check(!added, "Tu choi ID trung");
    if (!added)
    {
        delete duplicate;
    }
    check(!repo.add(repo.findById("ROM0001")), "Tu choi con tro trung");
    Room* invalidId = createRoom("MOV0004", "Sai ID");
    added = repo.add(invalidId);
    check(!added, "Tu choi ID sai tien to");
    if (!added)
    {
        delete invalidId;
    }
    Room* invalidName = createRoom("ROM0004", "");
    added = repo.add(invalidName);
    check(!added, "Tu choi ten rong");
    if (!added)
    {
        delete invalidName;
    }
    Room* invalidSize = createRoom("ROM0004", "Sai kich thuoc", 0, 12);
    added = repo.add(invalidSize);
    check(!added, "Tu choi so hang khong duong");
    if (!added)
    {
        delete invalidSize;
    }
    invalidSize = createRoom("ROM0004", "Sai kich thuoc", 8, -1);
    added = repo.add(invalidSize);
    check(!added, "Tu choi so cot khong duong");
    if (!added)
    {
        delete invalidSize;
    }
    check(repo.count() == 3, "Count khong doi sau add loi");
}

void testUpdate(IRepository<Room>& repo)
{
    cout << "\n===== TEST UPDATE =====" << endl;
    Room* room = repo.findById("ROM0001");
    if (room == nullptr)
    {
        check(false, "Co phong de update");
        return;
    }
    Seat seat("ROM0001", "A1", 8, 12, SeatType::STANDARD);
    Showtime showtime("SHW0001", "MOV0001", "ROM0001",
                      "2026-12-01 10:00", "2026-12-01 12:00",
                      ShowtimeStatus::SCHEDULED);
    check(room->linkSeat(&seat), "Lien ket Seat");
    check(room->linkShowtime(&showtime), "Lien ket Showtime");
    Room changed("ROM0001", "Phong moi", 9, 13, false);
    check(repo.update(changed), "Doi ten, trang thai va tang kich thuoc");
    check(room->getName() == "Phong moi" && !room->isActive() &&
          room->getRowCount() == 9 && room->getColumnCount() == 13,
          "Thuoc tinh sau update");
    check(repo.findById("ROM0001") == room && room->getRoomId() == "ROM0001" &&
          room->findSeat("A1") == &seat && room->findShowtime("SHW0001") == &showtime &&
          seat.getRoom() == room && showtime.getRoom() == room,
          "Giu ID, dia chi va lien ket hai chieu");
    Room invalidShrink("ROM0001", "Khong duoc doi", 7, 11, true);
    check(!repo.update(invalidShrink), "Tu choi thu nho khi ghe vuot pham vi");
    check(room->getName() == "Phong moi" && !room->isActive() &&
          room->getRowCount() == 9 && room->getColumnCount() == 13,
          "Update that bai khong doi thuoc tinh");
    Room validShrink("ROM0001", "Phong gon", 8, 12, false);
    check(repo.update(validShrink), "Thu nho hop le voi ghe o bien");
    check(room->getRowCount() == 8 && room->getColumnCount() == 12 &&
          room->findSeat("A1") == &seat && seat.getRoom() == room,
          "Ghe con lien ket sau thu nho");
    Room missing("ROM9999", "Khong ton tai", 8, 12, true);
    Room invalid("ROM0001", "", 8, 12, true);
    check(!repo.update(missing), "Tu choi update ID khong ton tai");
    check(!repo.update(invalid), "Tu choi update du lieu sai");
    check(room->getName() == "Phong gon" && !room->isActive(),
          "Khong doi Room sau update loi");
    check(!repo.removeById("ROM0001"), "Khong xoa Room con Seat va Showtime");
    check(room->unlinkSeat(&seat) && seat.getRoom() == nullptr,
          "Unlink Seat hai chieu");
    check(!repo.removeById("ROM0001"), "Khong xoa Room con Showtime");
    check(room->unlinkShowtime(&showtime) && showtime.getRoom() == nullptr,
          "Unlink Showtime hai chieu");
}

void testRemove(IRepository<Room>& repo)
{
    cout << "\n===== TEST REMOVE =====" << endl;
    check(repo.removeById("ROM0002"), "Xoa Room ton tai khong lien ket");
    check(repo.findById("ROM0002") == nullptr && repo.count() == 2,
          "Room da xoa va count giam");
    check(!repo.removeById("ROM0002"), "Khong xoa lai Room");
    check(!repo.removeById("ROM9999"), "Khong xoa ID khong ton tai");
    check(repo.count() == 2, "Count khong doi sau remove loi");
}

void testSave(IRepository<Room>& repo)
{
    cout << "\n===== TEST SAVE =====" << endl;
    check(repo.save(), "Save thanh cong");
    RoomRepository reloaded(TEST_FILE);
    IRepository<Room>& loaded = reloaded;
    check(loaded.load(), "Load lai file da save");
    check(loaded.count() == 2, "Count sau load lai");
    Room* first = loaded.findById("ROM0001");
    Room* third = loaded.findById("ROM0003");
    check(first != nullptr && first->getName() == "Phong gon" &&
          first->getRowCount() == 8 && first->getColumnCount() == 12 &&
          !first->isActive(), "Du lieu update duoc luu");
    check(third != nullptr && third->getName() == "Phong 3" &&
          third->isActive(), "Du lieu add duoc luu");
    check(loaded.findById("ROM0002") == nullptr, "Room da xoa khong duoc luu");
    check(first != nullptr && first->getSeatCount() == 0 &&
          first->getShowtimeCount() == 0,
          "rooms.txt khong luu lien ket Seat Showtime");
}

void testInvalidLoad()
{
    cout << "\n===== TEST INVALID LOAD =====" << endl;
    const string invalidLines[] = {
        "ROM0001|Phong 1|8|12\n",
        "ROM0001|Phong 1|abc|12|1\n",
        "ROM0001|Phong 1|8|12x|1\n",
        "ROM0001|Phong 1|8|12|2\n",
        "MOV0001|Phong 1|8|12|1\n",
        "ROM0001|Phong 1|0|12|1\n",
        "ROM0001||8|12|1\n",
        "ROM0001|Phong 1|8|12|1\nROM0001|Phong 2|8|12|1\n"
    };
    for (int i = 0; i < 8; i++)
    {
        writeTestFile(INVALID_FILE,
                      "ROM0009|Hop le|8|12|1\n" + invalidLines[i]);
        RoomRepository concrete(INVALID_FILE);
        IRepository<Room>& repo = concrete;
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
              repo.findById("ROM0009") == nullptr,
              "Load loi khong giu du lieu tam " + to_string(i + 1));
    }
    remove(MISSING_FILE.c_str());
    RoomRepository missing(MISSING_FILE);
    check(!missing.load() && !missing.isLoaded() && missing.count() == 0,
          "File thieu tra false va khong load");
    writeTestFile(INVALID_FILE, "");
    RoomRepository empty(INVALID_FILE);
    check(empty.load() && empty.isLoaded() && empty.count() == 0,
          "File rong load hop le");
}

void testPolymorphism()
{
    cout << "\n===== TEST POLYMORPHISM =====" << endl;
    IRepository<Room>* repo = new RoomRepository(TEST_FILE);
    check(repo->load() && repo->isLoaded() && repo->count() == 2 &&
          repo->findById("ROM0001") != nullptr,
          "Load va query qua con tro interface");
    delete repo;
    check(true, "Delete qua con tro interface");
}

int main()
{
    try
    {
        writeTestFile(TEST_FILE,
                      "ROM0001|Phong 1|8|12|1\n"
                      "ROM0002|Phong 2|6|10|0\n");
        testInterface();
        {
            RoomRepository concrete(TEST_FILE);
            IRepository<Room>& repo = concrete;
            testBeforeLoad(repo);
            testLoadAndQuery(repo);
            testAdd(repo);
            testUpdate(repo);
            testRemove(repo);
            testSave(repo);
        }
        testPolymorphism();
        testInvalidLoad();
    }
    catch (const exception& e)
    {
        failed++;
        cout << "\n[ERROR] " << e.what() << endl;
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
