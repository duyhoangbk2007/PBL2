
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "../src/business/SeatService.h"

using namespace std;

int passed = 0;
int failed = 0;

const string USER_FILE = "test_seat_service_users_temp.txt";
const string ROOM_FILE = "test_seat_service_rooms_temp.txt";
const string SEAT_FILE = "test_seat_service_seats_temp.txt";
const string PERMISSION_FILE = "test_seat_service_permissions_temp.txt";

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
    ofstream out(path);
    out << content;
}

void cleanFiles()
{
    remove(USER_FILE.c_str());
    remove(ROOM_FILE.c_str());
    remove(SEAT_FILE.c_str());
    remove(PERMISSION_FILE.c_str());
}

void prepareFiles()
{
    cleanFiles();

    writeFile(USER_FILE,
        "USR0001|admin|123456|Quan tri vien|ADMIN|1\n"
        "USR0002|staff|123456|Nhan vien|STAFF|1\n");

    writeFile(ROOM_FILE,
        "ROM0001|Phong 1|8|12|1\n"
        "ROM0002|Phong 2|5|10|1\n");

    writeFile(SEAT_FILE, "");
    writeFile(PERMISSION_FILE,
        "ADMIN|SEAT_VIEW\nADMIN|SEAT_ADD\nADMIN|SEAT_UPDATE\nADMIN|SEAT_DELETE\n"
        "STAFF|SEAT_VIEW\n");
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

Seat* createSeat(
    const string& roomId,
    const string& seatId,
    int row,
    int column,
    SeatType type = SeatType::STANDARD)
{
    return new Seat(roomId, seatId, row, column, type);
}

void testSeatService()
{
    cout << "\n===== TEST SEAT SERVICE =====\n";

    prepareFiles();

    {
        UserRepository userRepo(USER_FILE);
        RolePermissionRepository permissionRepo(PERMISSION_FILE);
        AuthService auth(userRepo, permissionRepo);
        RoomRepository roomRepo(ROOM_FILE);
        SeatRepository seatRepo(SEAT_FILE, roomRepo);
        SeatService service(seatRepo, auth);

        check("Load users", userRepo.load(), true);
        check("Load permissions", permissionRepo.load(), true);
        check("Save truoc load", service.save(), false);
        check("Khong load Seat truoc Room", service.load(), false);
        check("Load rooms", roomRepo.load(), true);
        check("Load seats", service.load(), true);
        check("Khong load seats lan hai", service.load(), false);

        // 1. Chua dang nhap
        Seat* unauthSeat = createSeat("ROM0001", "A1", 1, 1);

        check("Chua login khong duoc them",
              throwsError([&]() {
                  service.addSeat(unauthSeat);
              }), true);

        delete unauthSeat;

        Seat unauthUpdate(
            "ROM0001", "A1", 1, 1, SeatType::VIP);

        check("Chua login khong duoc sua",
              throwsError([&]() {
                  service.updateSeat(unauthUpdate);
              }), true);

        check("Chua login khong duoc xoa",
              throwsError([&]() {
                  service.removeSeat("ROM0001", "A1");
              }), true);

        check("Chua login khong duoc tim",
              throwsError([&]() {
                  service.findSeatById("ROM0001", "A1");
              }), true);

        check("Chua login khong duoc getAt",
              throwsError([&]() {
                  service.getSeatAt(0);
              }), true);

        check("Chua login khong duoc count",
              throwsError([&]() {
                  service.getSeatCount();
              }), true);

        // 2. Staff chi duoc xem
        check("Staff login",
              auth.login("staff", "123456"), true);

        Seat* staffSeat = createSeat(
            "ROM0001", "A1", 1, 1);

        check("Staff khong duoc them",
              throwsError([&]() {
                  service.addSeat(staffSeat);
              }), true);

        delete staffSeat;

        Seat staffUpdate(
            "ROM0001", "A1", 1, 1, SeatType::VIP);

        check("Staff khong duoc sua",
              throwsError([&]() {
                  service.updateSeat(staffUpdate);
              }), true);

        check("Staff khong duoc xoa",
              throwsError([&]() {
                  service.removeSeat("ROM0001", "A1");
              }), true);

        check("Staff duoc dem ghe",
              service.getSeatCount() == 0, true);

        check("Staff duoc tim ghe",
              service.findSeatById("ROM0001", "A1")
                  == nullptr, true);

        check("Staff duoc getAt",
              service.getSeatAt(0) == nullptr, true);

        // 3. Admin quan ly ghe
        check("Admin login",
              auth.login("admin", "123456"), true);

        Room* room1 = roomRepo.findById("ROM0001");
        Room* room2 = roomRepo.findById("ROM0002");

        Seat* seat1 = createSeat(
            "ROM0001", "A1", 1, 1);

        check("Admin them A1",
              service.addSeat(seat1), true);

        check("Dem sau them",
              service.getSeatCount() == 1, true);

        check("Tim A1",
              service.findSeatById("ROM0001", "A1")
                  == seat1, true);

        check("GetAt 0",
              service.getSeatAt(0) == seat1, true);

        check("GetAt ngoai pham vi",
              service.getSeatAt(99) == nullptr, true);

        check("Seat lien ket Room",
              seat1->getRoom() == room1, true);

        check("Room chua Seat",
              room1->findSeat("A1") == seat1, true);

        check("Khong them lai cung con tro",
              service.addSeat(seat1), false);

        // 4. Kiem tra du lieu khong hop le
        Seat* duplicate = createSeat(
            "ROM0001", "A1", 2, 2);

        bool added = service.addSeat(duplicate);
        check("Khong them trung ID", added, false);
        if (!added) delete duplicate;

        Seat* duplicatePosition = createSeat(
            "ROM0001", "B1", 1, 1);

        added = service.addSeat(duplicatePosition);
        check("Khong them trung vi tri", added, false);
        if (!added) delete duplicatePosition;

        Seat* outside = createSeat(
            "ROM0001", "Z1", 9, 1);

        added = service.addSeat(outside);
        check("Khong them ngoai kich thuoc", added, false);
        if (!added) delete outside;

        Seat* missingRoom = createSeat(
            "ROM9999", "A1", 1, 1);

        added = service.addSeat(missingRoom);
        check("Khong them Room khong ton tai", added, false);
        if (!added) delete missingRoom;

        Seat* invalidType = createSeat(
            "ROM0001", "B1", 2, 1,
            static_cast<SeatType>(99));

        added = service.addSeat(invalidType);
        check("Khong them loai ghe sai", added, false);
        if (!added) delete invalidType;

        check("Khong them nullptr",
              service.addSeat(nullptr), false);

        // 5. Hai Room duoc trung seatId
        Seat* seat2 = createSeat(
            "ROM0002", "A1", 1, 1, SeatType::VIP);

        check("Them A1 vao Room 2",
              service.addSeat(seat2), true);

        check("Tim A1 Room 2",
              service.findSeatById("ROM0002", "A1")
                  == seat2, true);

        check("A1 Room 2 lien ket dung",
              seat2->getRoom() == room2, true);

        check("Tong so ghe",
              service.getSeatCount() == 2, true);

        // 6. Cap nhat loai ghe
        Seat updated(
            "ROM0001", "A1", 1, 1, SeatType::COUPLE);

        check("Admin update Seat",
              service.updateSeat(updated), true);

        check("Loai ghe sau update",
              seat1->getSeatType() == SeatType::COUPLE,
              true);

        Seat moved(
            "ROM0001", "A1", 2, 1, SeatType::VIP);

        check("Khong cho doi vi tri",
              service.updateSeat(moved), false);

        check("Giu loai ghe khi update loi",
              seat1->getSeatType() == SeatType::COUPLE,
              true);

        Seat missing(
            "ROM0001", "Z9", 1, 1, SeatType::VIP);

        check("Khong update Seat khong ton tai",
              service.updateSeat(missing), false);

        // 7. Luu du lieu
        check("Save Seat",
              service.save(), true);

        // 8. Xoa Seat
        check("Admin xoa Seat Room 2",
              service.removeSeat("ROM0002", "A1"), true);

        check("Room 2 khong con A1",
              room2->findSeat("A1") == nullptr, true);

        check("So ghe sau xoa",
              service.getSeatCount() == 1, true);

        check("Khong xoa Seat khong ton tai",
              service.removeSeat("ROM0002", "Z9"), false);

        check("Seat Room 1 van con",
              service.findSeatById("ROM0001", "A1")
                  == seat1, true);

        // 9. Dang xuat
        auth.logout();

        check("Logout mat quyen xem",
              throwsError([&]() {
                  service.getSeatCount();
              }), true);

        check("Logout mat quyen them",
              throwsError([&]() {
                  service.addSeat(nullptr);
              }), true);
    }

    // 10. Nap lai du lieu da save
    {
        RoomRepository roomRepo(ROOM_FILE);
        SeatRepository seatRepo(SEAT_FILE, roomRepo);

        check("Load Room lai", roomRepo.load(), true);
        check("Load Seat lai", seatRepo.load(), true);

        check("So ghe sau load lai",
              seatRepo.count() == 2, true);

        Seat* seat = seatRepo.findById("ROM0001", "A1");

        check("Khoi phuc A1",
              seat != nullptr, true);

        check("Khoi phuc loai COUPLE",
              seat != nullptr &&
              seat->getSeatType() == SeatType::COUPLE,
              true);

        check("Khoi phuc vi tri",
              seat != nullptr &&
              seat->getRow() == 1 &&
              seat->getColumn() == 1, true);

        check("Khoi phuc lien ket Room",
              seat != nullptr &&
              seat->getRoom() == roomRepo.findById("ROM0001"),
              true);

        check("Room 2 A1 co trong file da save",
              seatRepo.findById("ROM0002", "A1") != nullptr,
              true);
    }

    cleanFiles();
}

int main()
{
    testSeatService();

    cout << "\n===== KET QUA =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total : " << passed + failed << endl;

    if (failed == 0)
    {
        cout << "\nALL TESTS PASSED!\n";
        return 0;
    }

    cout << "\nSOME TESTS FAILED!\n";
    return 1;
}
