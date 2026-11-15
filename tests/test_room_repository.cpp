
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "../src/data_access/RoomRepository.h"
#include "../src/models/Seat.h"
#include "../src/models/Showtime.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_rooms_temp.txt";

// Kiem tra ket qua
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

// Ghi file du lieu mau
void writeTestFile(const string& content)
{
    ofstream file(TEST_FILE);
    file << content;
}

// Xoa file test
void removeTestFile()
{
    remove(TEST_FILE.c_str());
}

// Tao phong de test
Room* createRoom(
    const string& id,
    const string& name = "Phong 1",
    int rows = 8,
    int columns = 12,
    bool active = true)
{
    return new Room(id, name, rows, columns, active);
}

// 1. Test cac thao tac co ban
void testRoomRepository()
{
    cout << "\n===== TEST ROOM REPOSITORY =====\n";

    writeTestFile("");

    {
        RoomRepository repo(TEST_FILE);

        check("Chua load", repo.isLoaded(), false);
        check("Save truoc load", repo.save(), false);
        check("Load file rong", repo.load(), true);
        check("Da load", repo.isLoaded(), true);
        check("Danh sach rong", repo.count() == 0, true);

        Room* room1 = createRoom("ROM0001");

        check("Them phong hop le", repo.add(room1), true);
        check("So luong sau them", repo.count() == 1, true);

        check("Tim phong ton tai",
              repo.findById("ROM0001") == room1, true);

        check("Tim phong khong ton tai",
              repo.findById("ROM9999") == nullptr, true);

        check("Lay phong vi tri 0",
              repo.getAt(0) == room1, true);

        check("Lay phong ngoai pham vi",
              repo.getAt(100) == nullptr, true);

        check("Them lai cung con tro",
              repo.add(room1), false);

        Room* duplicate = createRoom("ROM0001", "Phong khac");
        bool added = repo.add(duplicate);

        check("Them trung ID", added, false);
        if (!added) delete duplicate;

        Room* invalidId = createRoom("ROM0000");
        added = repo.add(invalidId);

        check("Them ID sai", added, false);
        if (!added) delete invalidId;

        Room* invalidRows = createRoom("ROM0002", "Phong 2", 0, 12);
        added = repo.add(invalidRows);

        check("Them so hang bang 0", added, false);
        if (!added) delete invalidRows;

        Room* invalidColumns = createRoom("ROM0002", "Phong 2", 8, -1);
        added = repo.add(invalidColumns);

        check("Them so cot am", added, false);
        if (!added) delete invalidColumns;

        Room* invalidName = createRoom("ROM0002", "");
        added = repo.add(invalidName);

        check("Them ten rong", added, false);
        if (!added) delete invalidName;

        Room* invalidSeparator = createRoom("ROM0002", "Phong|2");
        added = repo.add(invalidSeparator);

        check("Ten chua dau phan cach", added, false);
        if (!added) delete invalidSeparator;

        check("Them nullptr", repo.add(nullptr), false);
        check("So luong khong doi", repo.count() == 1, true);

        // Cap nhat phong
        Room updated("ROM0001", "Phong VIP", 10, 15, false);

        check("Cap nhat phong", repo.update(updated), true);

        Room* afterUpdate = repo.findById("ROM0001");

        check("Ten sau update",
              afterUpdate->getName() == "Phong VIP", true);

        check("So hang sau update",
              afterUpdate->getRowCount() == 10, true);

        check("So cot sau update",
              afterUpdate->getColumnCount() == 15, true);

        check("Trang thai sau update",
              !afterUpdate->isActive(), true);

        Room notFound("ROM9999", "Khong ton tai", 8, 12, true);

        check("Update phong khong ton tai",
              repo.update(notFound), false);

        Room invalidUpdate("ROM0001", "Phong sai", -1, 12, true);

        check("Update du lieu sai",
              repo.update(invalidUpdate), false);

        check("Giu du lieu khi update loi",
              afterUpdate->getName() == "Phong VIP" &&
              afterUpdate->getRowCount() == 10, true);

        // Luu file
        check("Save du lieu", repo.save(), true);

        // Khong cho load lai
        check("Khong load lai khi da loaded",
              repo.load(), false);

        check("Du lieu van con sau load lai",
              repo.findById("ROM0001") == room1, true);

        // Xoa phong
        check("Xoa phong ton tai",
              repo.removeById("ROM0001"), true);

        check("Danh sach rong sau xoa",
              repo.count() == 0, true);

        check("Xoa phong khong ton tai",
              repo.removeById("ROM9999"), false);
    }

    // Tao repository moi de doc lai file da save
    {
        RoomRepository repo(TEST_FILE);

        check("Load lai bang repository moi",
              repo.load(), true);

        check("So luong sau load lai",
              repo.count() == 1, true);

        Room* room = repo.findById("ROM0001");

        check("Phong duoc khoi phuc",
              room != nullptr, true);

        check("Ten phong duoc khoi phuc",
              room != nullptr && room->getName() == "Phong VIP",
              true);

        check("Kich thuoc duoc khoi phuc",
              room != nullptr &&
              room->getRowCount() == 10 &&
              room->getColumnCount() == 15, true);

        check("Trang thai duoc khoi phuc",
              room != nullptr && !room->isActive(), true);
    }

    removeTestFile();
}

// 2. Test rang buoc ghe
void testRoomWithSeats()
{
    cout << "\n===== TEST ROOM WITH SEATS =====\n";

    writeTestFile("");

    {
        RoomRepository repo(TEST_FILE);
        check("Load de test ghe", repo.load(), true);

        Room* room = createRoom("ROM0001");
        check("Them phong de lien ket ghe", repo.add(room), true);

        // Seat khong thuoc quyen so huu cua RoomRepository
        Seat seat1("ROM0001", "A01", 1, 1, SeatType::STANDARD);
        Seat seat2("ROM0001", "H12", 8, 12, SeatType::VIP);

        check("Lien ket ghe A01", room->linkSeat(&seat1), true);
        check("Lien ket ghe H12", room->linkSeat(&seat2), true);
        check("So ghe da lien ket", room->getSeatCount() == 2, true);

        check("Lay ghe theo chi so",
              room->getSeatAt(1) == &seat2, true);

        check("Lay ghe ngoai pham vi",
              room->getSeatAt(100) == nullptr, true);

        Room smaller("ROM0001", "Phong nho", 6, 10, true);

        check("Khong thu nho khi ghe vuot gioi han",
              repo.update(smaller), false);

        check("Giu kich thuoc khi update that bai",
              room->getRowCount() == 8 &&
              room->getColumnCount() == 12, true);

        Room larger("ROM0001", "Phong lon", 10, 15, true);

        check("Mo rong phong",
              repo.update(larger), true);

        check("Kich thuoc sau mo rong",
              room->getRowCount() == 10 &&
              room->getColumnCount() == 15, true);

        Room exact("ROM0001", "Phong vua du", 8, 12, true);

        check("Thu nho vua du cho cac ghe",
              repo.update(exact), true);

        check("Khong xoa phong con ghe",
              repo.removeById("ROM0001"), false);

        check("Phong van ton tai",
              repo.findById("ROM0001") == room, true);
    }

    removeTestFile();
}

// 3. Test rang buoc suat chieu
void testRoomWithShowtime()
{
    cout << "\n===== TEST ROOM WITH SHOWTIME =====\n";

    writeTestFile("");

    {
        RoomRepository repo(TEST_FILE);
        check("Load de test suat chieu", repo.load(), true);

        Room* room = createRoom("ROM0002");
        check("Them phong co suat chieu", repo.add(room), true);

        Showtime showtime(
            "SHW0001",
            "MOV0001",
            "ROM0002",
            "2026-12-01 18:00",
            "2026-12-01 20:00",
            ShowtimeStatus::SCHEDULED
        );

        check("Lien ket suat chieu",
              room->linkShowtime(&showtime), true);

        check("So suat chieu",
              room->getShowtimeCount() == 1, true);

        check("Khong xoa phong con suat chieu",
              repo.removeById("ROM0002"), false);

        check("Phong van ton tai",
              repo.findById("ROM0002") == room, true);
    }

    removeTestFile();
}

// 4. Test file khong hop le
void testInvalidFiles()
{
    cout << "\n===== TEST INVALID FILES =====\n";

    removeTestFile();

    {
        RoomRepository repo(TEST_FILE);

        check("Load file khong ton tai", repo.load(), false);
        check("Van chua loaded", repo.isLoaded(), false);
        check("Save khi chua load", repo.save(), false);
    }

    // Sai so luong truong
    writeTestFile("ROM0001|Phong 1|8|12\n");

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File thieu truong nem exception", threw, true);
        check("Khong loaded khi loi", repo.isLoaded(), false);
        check("Khong giu du lieu loi", repo.count() == 0, true);
    }

    // Sai dinh dang so
    writeTestFile("ROM0001|Phong 1|abc|12|1\n");

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File sai dinh dang so", threw, true);
    }

    // So co ky tu thua
    writeTestFile("ROM0001|Phong 1|8abc|12|1\n");

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File so co ky tu thua", threw, true);
    }

    // Trang thai khong hop le
    writeTestFile("ROM0001|Phong 1|8|12|2\n");

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File sai trang thai", threw, true);
    }

    // ID khong hop le
    writeTestFile("ROM0000|Phong 1|8|12|1\n");

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File ID khong hop le", threw, true);
    }

    // Trung ID
    writeTestFile(
        "ROM0001|Phong 1|8|12|1\n"
        "ROM0001|Phong 2|10|15|1\n"
    );

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File trung ID nem exception", threw, true);
        check("Khong loaded khi trung ID", repo.isLoaded(), false);
        check("Khong giu du lieu doc do", repo.count() == 0, true);
    }

    // Dong dau hop le, dong sau sai
    writeTestFile(
        "ROM0001|Phong 1|8|12|1\n"
        "ROM0002|Phong 2|0|12|1\n"
    );

    {
        RoomRepository repo(TEST_FILE);
        bool threw = false;

        try { repo.load(); }
        catch (...) { threw = true; }

        check("File co dong sai nem exception", threw, true);
        check("Rollback du lieu khi load loi",
              repo.count() == 0 && !repo.isLoaded(), true);
    }

    removeTestFile();
}

// 5. Test hanh vi khi chua load
void testBeforeLoad()
{
    cout << "\n===== TEST BEFORE LOAD =====\n";

    removeTestFile();

    {
        RoomRepository repo(TEST_FILE);

        Room* room = createRoom("ROM0001");

        bool added = repo.add(room);

        check("Khong them khi chua load", added, false);

        if (!added) delete room;

        Room updated("ROM0001", "Phong moi", 8, 12, true);

        check("Khong update khi chua load",
              repo.update(updated), false);

        check("Khong xoa khi chua load",
              repo.removeById("ROM0001"), false);
    }

    removeTestFile();
}

int main()
{
    cout << "====================================\n";
    cout << "       ROOM REPOSITORY TESTS\n";
    cout << "====================================\n";

    testRoomRepository();
    testRoomWithSeats();
    testRoomWithShowtime();
    testInvalidFiles();
    testBeforeLoad();

    cout << "\n====================================\n";
    cout << "             SUMMARY\n";
    cout << "====================================\n";

    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total : " << passed + failed << endl;

    removeTestFile();

    if (failed == 0)
    {
        cout << "\nALL TESTS PASSED!\n";
        return 0;
    }

    cout << "\nSOME TESTS FAILED!\n";
    return 1;
}
