#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "../src/business/RoomService.h"
#include "../src/models/Seat.h"
#include "../src/models/Showtime.h"

using namespace std;

int passed = 0;
int failed = 0;
const string USER_FILE = "test_room_service_users_temp.txt";
const string ROOM_FILE = "test_room_service_rooms_temp.txt";

void check(const string& name, bool actual, bool expected)
{
    if (actual == expected)
    {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name << " | Expected: " << expected
             << " | Actual: " << actual << endl;
        failed++;
    }
}

void writeFile(const string& path, const string& content)
{
    ofstream out(path);
    out << content;
}

void cleanFiles()
{
    remove(USER_FILE.c_str());
    remove(ROOM_FILE.c_str());
}

// Kiem tra mot thao tac co nem exception hay khong.
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

void testRoomService()
{
    cout << "\n===== TEST ROOM SERVICE =====\n";
    cleanFiles();
    writeFile(USER_FILE,
        "USR0001|admin|123456|Quan tri vien|ADMIN|1\n"
        "USR0002|staff|123456|Nhan vien|STAFF|1\n");
    writeFile(ROOM_FILE, "");

    {
        UserRepository userRepo(USER_FILE);
        AuthService auth(userRepo);
        RoomRepository roomRepo(ROOM_FILE);
        RoomService service(roomRepo, auth);

        check("Load users", userRepo.load(), true);
        check("Save room truoc load", service.save(), false);
        check("Load rooms", service.load(), true);
        check("Khong load rooms lan hai", service.load(), false);

        // 1. Chua dang nhap
        Room* unauthRoom = new Room("ROM0001", "Phong 1", 8, 12, true);
        check("Chua login khong duoc them",
              throwsError([&]() { service.addRoom(unauthRoom); }), true);
        delete unauthRoom;

        check("Chua login khong duoc tim",
              throwsError([&]() { service.findRoomById("ROM0001"); }), true);
        check("Chua login khong duoc getAt",
              throwsError([&]() { service.getRoomAt(0); }), true);
        check("Chua login khong duoc count",
              throwsError([&]() { service.getRoomCount(); }), true);

        // 2. Staff chi co quyen xem
        check("Staff login", auth.login("staff", "123456"), true);
        Room* staffRoom = new Room("ROM0001", "Phong 1", 8, 12, true);
        check("Staff khong duoc them",
              throwsError([&]() { service.addRoom(staffRoom); }), true);
        delete staffRoom;
        Room staffUpdate("ROM0001", "Sua", 8, 12, true);
        check("Staff khong duoc sua",
              throwsError([&]() { service.updateRoom(staffUpdate); }), true);
        check("Staff khong duoc xoa",
              throwsError([&]() { service.removeRoom("ROM0001"); }), true);
        check("Staff duoc dem phong", service.getRoomCount() == 0, true);
        check("Staff tim phong khong co", service.findRoomById("ROM0001") == nullptr, true);
        check("Staff getAt rong", service.getRoomAt(0) == nullptr, true);

        // 3. Admin quan ly phong
        check("Admin login", auth.login("admin", "123456"), true);
        Room* room1 = new Room("ROM0001", "Phong 1", 8, 12, true);
        check("Admin them phong", service.addRoom(room1), true);
        check("Dem phong sau them", service.getRoomCount() == 1, true);
        check("Tim phong theo ID", service.findRoomById("ROM0001") == room1, true);
        check("Lay phong tai vi tri 0", service.getRoomAt(0) == room1, true);
        check("GetAt ngoai pham vi", service.getRoomAt(99) == nullptr, true);
        check("Tim ID khong ton tai", service.findRoomById("ROM9999") == nullptr, true);
        check("Khong them lai cung con tro", service.addRoom(room1), false);

        Room* duplicate = new Room("ROM0001", "Phong trung", 8, 12, true);
        bool added = service.addRoom(duplicate);
        check("Khong them trung ID", added, false);
        if (!added) delete duplicate;

        Room* invalid = new Room("ROM0002", "", 8, 12, true);
        added = service.addRoom(invalid);
        check("Khong them ten rong", added, false);
        if (!added) delete invalid;
        check("Khong them nullptr", service.addRoom(nullptr), false);

        Room updated("ROM0001", "Phong VIP", 10, 15, true);
        check("Admin sua phong", service.updateRoom(updated), true);
        check("Ten da thay doi", room1->getName() == "Phong VIP", true);
        check("Kich thuoc da thay doi",
              room1->getRowCount() == 10 && room1->getColumnCount() == 15, true);

        Room changedActive("ROM0001", "Phong VIP", 10, 15, false);
        check("Khong doi active qua update", service.updateRoom(changedActive), false);
        check("Active van giu nguyen", room1->isActive(), true);

        Room missing("ROM9999", "Khong co", 8, 12, true);
        check("Khong update ID khong ton tai", service.updateRoom(missing), false);
        Room invalidUpdate("ROM0001", "", 10, 15, true);
        check("Khong update ten rong", service.updateRoom(invalidUpdate), false);
        check("Ten khong bi thay doi khi update loi", room1->getName() == "Phong VIP", true);

        // 4. Rang buoc ghe: phong co ghe khong duoc thu nho qua vi tri ghe
        Seat seat("ROM0001", "J15", 10, 15, SeatType::STANDARD);
        check("Lien ket ghe", room1->linkSeat(&seat), true);
        Room smaller("ROM0001", "Phong nho", 8, 12, true);
        check("Khong thu nho vuot ghe", service.updateRoom(smaller), false);
        check("Khong xoa phong co ghe", service.removeRoom("ROM0001"), false);

        // 5. Rang buoc suat chieu: khong doi kich thuoc neu co suat chieu
        Room* room2 = new Room("ROM0002", "Phong 2", 8, 12, true);
        check("Them phong 2", service.addRoom(room2), true);
        Showtime showtime("SHW0001", "MOV0001", "ROM0002",
                          "2026-12-01 18:00", "2026-12-01 20:00",
                          ShowtimeStatus::SCHEDULED);
        check("Lien ket suat chieu", room2->linkShowtime(&showtime), true);
        Room resized("ROM0002", "Phong 2", 9, 12, true);
        check("Khong doi kich thuoc khi co showtime", service.updateRoom(resized), false);
        Room renamed("ROM0002", "Phong 2 VIP", 8, 12, true);
        check("Duoc doi ten khi co showtime", service.updateRoom(renamed), true);
        check("Khong xoa phong co showtime", service.removeRoom("ROM0002"), false);

        // 6. Xoa phong khong co lien ket
        Room* room3 = new Room("ROM0003", "Phong 3", 5, 5, true);
        check("Them phong 3", service.addRoom(room3), true);
        check("Xoa phong khong co lien ket", service.removeRoom("ROM0003"), true);
        check("Khong xoa ID khong ton tai", service.removeRoom("ROM9999"), false);
        check("Dem phong sau xoa", service.getRoomCount() == 2, true);

        // 7. Luu va doc lai
        check("Save rooms", service.save(), true);
        auth.logout();
        check("Logout mat quyen xem",
              throwsError([&]() { service.getRoomCount(); }), true);
    }

    {
        RoomRepository reloaded(ROOM_FILE);
        check("Load lai file rooms", reloaded.load(), true);
        check("So phong sau load lai", reloaded.count() == 2, true);
        Room* room = reloaded.findById("ROM0001");
        check("Du lieu ten duoc luu", room != nullptr && room->getName() == "Phong VIP", true);
        check("Du lieu kich thuoc duoc luu", room != nullptr &&
              room->getRowCount() == 10 && room->getColumnCount() == 15, true);
        check("Phong 2 duoc luu", reloaded.findById("ROM0002") != nullptr, true);
        check("Phong 3 da bi xoa", reloaded.findById("ROM0003") == nullptr, true);
    }

    cleanFiles();
}

int main()
{
    testRoomService();
    cout << "\n===== KET QUA =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    return failed == 0 ? 0 : 1;
}
