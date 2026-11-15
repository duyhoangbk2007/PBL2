
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/SeatRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

const string ROOM_FILE = "test_seat_rooms_temp.txt";
const string SEAT_FILE = "test_seat_seats_temp.txt";

void check(const string& name, bool actual, bool expected)
{
    if (actual == expected)
    {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name
             << " | Expected: " << expected
             << " | Actual: " << actual << endl;
        failed++;
    }
}

void writeFile(const string& path, const string& content)
{
    ofstream file(path);
    file << content;
}

void cleanFiles()
{
    remove(ROOM_FILE.c_str());
    remove(SEAT_FILE.c_str());
}

void prepareFiles(const string& seats = "")
{
    cleanFiles();
    writeFile(ROOM_FILE,
        "ROM0001|Phong 1|8|12|1\n"
        "ROM0002|Phong 2|5|10|1\n");
    writeFile(SEAT_FILE, seats);
}

Seat* createSeat(
    const string& roomId,
    const string& seatId,
    int row,
    int column,
    SeatType type = SeatType::STANDARD)
{
    return new Seat(roomId, seatId, row, column, type);
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

// 1. Kiem tra CRUD va lien ket Room - Seat
void testCRUD()
{
    cout << "\n===== TEST SEAT CRUD =====\n";
    prepareFiles();

    RoomRepository roomRepo(ROOM_FILE);
    SeatRepository seatRepo(SEAT_FILE, roomRepo);

    check("Load Room", roomRepo.load(), true);
    check("Load Seat", seatRepo.load(), true);
    check("Seat da loaded", seatRepo.isLoaded(), true);
    check("Danh sach ban dau rong", seatRepo.count() == 0, true);
    check("Khong load Seat lan hai", seatRepo.load(), false);

    Room* room1 = roomRepo.findById("ROM0001");
    Room* room2 = roomRepo.findById("ROM0002");

    Seat* seat1 = createSeat("ROM0001", "A1", 1, 1);
    check("Them A1", seatRepo.add(seat1), true);
    check("Tim A1", seatRepo.findById("ROM0001", "A1") == seat1, true);
    check("Seat lien ket Room", seat1->getRoom() == room1, true);
    check("Room chua A1", room1->findSeat("A1") == seat1, true);
    check("So ghe Room 1", room1->getSeatCount() == 1, true);

    Seat* seat2 = createSeat("ROM0001", "A2", 1, 2, SeatType::VIP);
    check("Them A2", seatRepo.add(seat2), true);
    check("So ghe sau them", seatRepo.count() == 2, true);
    check("GetAt 0", seatRepo.getAt(0) == seat1, true);
    check("GetAt ngoai pham vi", seatRepo.getAt(99) == nullptr, true);

    // SeatId chi duy nhat trong tung Room
    Seat* sameId = createSeat("ROM0002", "A1", 1, 1);
    check("Hai Room duoc trung seatId", seatRepo.add(sameId), true);
    check("Tim A1 cua Room 2",
          seatRepo.findById("ROM0002", "A1") == sameId, true);
    check("Lien ket Room 2", sameId->getRoom() == room2, true);

    check("Khong them lai cung con tro", seatRepo.add(seat1), false);

    Seat* duplicate = createSeat("ROM0001", "A1", 2, 2);
    bool added = seatRepo.add(duplicate);
    check("Khong them trung khoa", added, false);
    if (!added) delete duplicate;

    Seat* duplicatePosition = createSeat("ROM0001", "B1", 1, 1);
    added = seatRepo.add(duplicatePosition);
    check("Khong them trung vi tri", added, false);
    if (!added) delete duplicatePosition;

    Seat* outside = createSeat("ROM0001", "Z1", 9, 1);
    added = seatRepo.add(outside);
    check("Khong them ghe ngoai phong", added, false);
    if (!added) delete outside;

    Seat* missingRoom = createSeat("ROM9999", "A1", 1, 1);
    added = seatRepo.add(missingRoom);
    check("Khong them ghe khi Room khong ton tai", added, false);
    if (!added) delete missingRoom;

    Seat* invalidRow = createSeat("ROM0001", "B1", 0, 1);
    added = seatRepo.add(invalidRow);
    check("Khong them hang bang 0", added, false);
    if (!added) delete invalidRow;

    Seat* invalidType = createSeat(
        "ROM0001", "B1", 2, 1, static_cast<SeatType>(99));
    added = seatRepo.add(invalidType);
    check("Khong them loai ghe sai", added, false);
    if (!added) delete invalidType;

    check("Khong them nullptr", seatRepo.add(nullptr), false);
    check("Khong tim thay Seat", 
          seatRepo.findById("ROM0001", "Z9") == nullptr, true);

    Seat updated("ROM0001", "A1", 1, 1, SeatType::COUPLE);
    check("Update loai ghe", seatRepo.update(updated), true);
    check("Loai ghe da thay doi",
          seat1->getSeatType() == SeatType::COUPLE, true);

    Seat moved("ROM0001", "A1", 2, 1, SeatType::VIP);
    check("Khong doi vi tri qua update", seatRepo.update(moved), false);
    check("Loai ghe khong bi doi khi update loi",
          seat1->getSeatType() == SeatType::COUPLE, true);

    Seat notFound("ROM0001", "Z9", 2, 1, SeatType::VIP);
    check("Khong update ghe khong ton tai",
          seatRepo.update(notFound), false);

    check("Save Seat", seatRepo.save(), true);

    check("Xoa A2", seatRepo.removeById("ROM0001", "A2"), true);
    check("Room khong con A2", room1->findSeat("A2") == nullptr, true);
    check("So ghe Room 1 sau xoa", room1->getSeatCount() == 1, true);
    check("Khong xoa ID khong ton tai",
          seatRepo.removeById("ROM0001", "Z9"), false);
    check("So ghe con lai", seatRepo.count() == 2, true);
}

// 2. Kiem tra luu va nap lai
void testSaveLoad()
{
    cout << "\n===== TEST SAVE / LOAD =====\n";

    // Doc lai file da save o testCRUD
    RoomRepository roomRepo(ROOM_FILE);
    SeatRepository seatRepo(SEAT_FILE, roomRepo);

    check("Load Room lai", roomRepo.load(), true);
    check("Load Seat lai", seatRepo.load(), true);

    check("So ghe doc tu file", seatRepo.count() == 3, true);

    Seat* seat = seatRepo.findById("ROM0001", "A1");
    check("Khoi phuc A1", seat != nullptr, true);
    check("Khoi phuc loai COUPLE",
          seat != nullptr && seat->getSeatType() == SeatType::COUPLE,
          true);
    check("Khoi phuc vi tri",
          seat != nullptr && seat->getRow() == 1 &&
          seat->getColumn() == 1, true);

    Room* room = roomRepo.findById("ROM0001");
    check("Lien ket lai sau load",
          seat != nullptr && seat->getRoom() == room, true);

    check("A2 van co trong file da save",
          seatRepo.findById("ROM0001", "A2") != nullptr, true);

    check("A1 cua Room 2 van ton tai",
          seatRepo.findById("ROM0002", "A1") != nullptr, true);
}

// 3. Kiem tra load truoc Room
void testBeforeLoad()
{
    cout << "\n===== TEST BEFORE LOAD =====\n";
    prepareFiles();

    RoomRepository roomRepo(ROOM_FILE);
    SeatRepository seatRepo(SEAT_FILE, roomRepo);

    check("Seat chua loaded", seatRepo.isLoaded(), false);
    check("Khong load Seat truoc Room", seatRepo.load(), false);
    check("Khong save khi chua load", seatRepo.save(), false);

    Seat* seat = createSeat("ROM0001", "A1", 1, 1);
    bool added = seatRepo.add(seat);
    check("Khong them khi chua load", added, false);
    if (!added) delete seat;

    Seat updated("ROM0001", "A1", 1, 1, SeatType::VIP);
    check("Khong update khi chua load", seatRepo.update(updated), false);
    check("Khong xoa khi chua load",
          seatRepo.removeById("ROM0001", "A1"), false);

    check("Load Room sau do", roomRepo.load(), true);
    check("Load Seat sau Room", seatRepo.load(), true);
}

// 4. Kiem tra file sai dinh dang
void testInvalidFiles()
{
    cout << "\n===== TEST INVALID FILES =====\n";

    const string invalidData[] = {
        "ROM0001|A1|1|1\n",
        "ROM0001|A1|abc|1|STANDARD\n",
        "ROM0001|A1|1abc|1|STANDARD\n",
        "ROM0001|A1|1|1|UNKNOWN\n",
        "ROM0000|A1|1|1|STANDARD\n",
        "ROM0001|A1|0|1|STANDARD\n",
        "ROM0001|A1|1|1|STANDARD\n"
        "ROM0001|A1|1|2|VIP\n"
    };

    const string names[] = {
        "Thieu truong",
        "Hang khong phai so",
        "Hang co ky tu thua",
        "Loai ghe sai",
        "Room ID sai",
        "Hang bang 0",
        "Trung khoa ghe"
    };

    for (int i = 0; i < 7; i++)
    {
        prepareFiles(invalidData[i]);

        RoomRepository roomRepo(ROOM_FILE);
        SeatRepository seatRepo(SEAT_FILE, roomRepo);
        roomRepo.load();

        check(names[i] + " nem exception",
              throwsError([&]() { seatRepo.load(); }), true);

        check(names[i] + " khong loaded",
              seatRepo.isLoaded(), false);

        check(names[i] + " khong giu Seat",
              seatRepo.count() == 0, true);
    }
}

// 5. Kiem tra rollback khi lien ket that bai
void testRollback()
{
    cout << "\n===== TEST LINK ROLLBACK =====\n";

    prepareFiles(
        "ROM0001|A1|1|1|STANDARD\n"
        "ROM0001|A2|1|2|VIP\n"
        "ROM9999|A3|1|3|STANDARD\n"
    );

    RoomRepository roomRepo(ROOM_FILE);
    SeatRepository seatRepo(SEAT_FILE, roomRepo);

    check("Load Room cho rollback", roomRepo.load(), true);
    Room* room = roomRepo.findById("ROM0001");

    check("Load Seat loi do thieu Room", seatRepo.load(), false);
    check("Seat khong loaded sau rollback", seatRepo.isLoaded(), false);
    check("Seat da duoc clear", seatRepo.count() == 0, true);
    check("Room khong con con tro Seat",
          room->getSeatCount() == 0, true);

    // Hai ghe trung vi tri trong cung phong
    writeFile(SEAT_FILE,
        "ROM0001|A1|1|1|STANDARD\n"
        "ROM0001|B1|1|1|VIP\n"
    );

    check("Load loi khi trung vi tri", seatRepo.load(), false);
    check("Rollback khi trung vi tri",
          room->getSeatCount() == 0 && seatRepo.count() == 0,
          true);
}

// 6. Kiem tra file khong ton tai
void testMissingFile()
{
    cout << "\n===== TEST MISSING FILE =====\n";

    prepareFiles();
    remove(SEAT_FILE.c_str());

    RoomRepository roomRepo(ROOM_FILE);
    SeatRepository seatRepo(SEAT_FILE, roomRepo);

    check("Load Room", roomRepo.load(), true);
    check("Khong load duoc file Seat thieu", seatRepo.load(), false);
    check("Khong loaded khi thieu file", seatRepo.isLoaded(), false);
}

int main()
{
    testCRUD();
    testSaveLoad();
    testBeforeLoad();
    testInvalidFiles();
    testRollback();
    testMissingFile();

    cleanFiles();

    cout << "\n===== KET QUA =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total : " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
