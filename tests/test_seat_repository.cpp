#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "../src/data_access/IRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/SeatRepository.h"

using namespace std;

int passed = 0;
int failed = 0;
const string ROOMS = "test_seat_repository_rooms.txt";
const string SEATS = "test_seat_repository_seats.txt";
const string MISSING = "test_seat_repository_missing.txt";
const string ROOM_DATA =
    "ROM0001|Phong 1|3|4|1\n"
    "ROM0002|Phong 2|2|3|1\n";
const string FIRST = "ROM0001|A01|1|1|STANDARD\n";
const string SECOND = "ROM0001|A02|1|2|VIP\n";

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << endl;
    (condition ? passed : failed)++;
}

void writeTestFile(const string& path, const string& data)
{
    ofstream file(path, ios::binary);
    if (!file || !(file << data))
    {
        throw runtime_error("Khong ghi duoc file test: " + path);
    }
}

string readTestFile(const string& path)
{
    ifstream file(path, ios::binary);
    if (!file)
    {
        throw runtime_error("Khong doc duoc file test: " + path);
    }
    string data{istreambuf_iterator<char>(file), istreambuf_iterator<char>()};
    string normalized;
    for (char ch : data)
    {
        if (ch != '\r')
        {
            normalized += ch;
        }
    }
    return normalized;
}

void prepare(const string& seats = "")
{
    writeTestFile(ROOMS, ROOM_DATA);
    writeTestFile(SEATS, seats);
}

Seat* makeSeat(const string& id, const string& roomId = "ROM0001",
               int row = 1, int column = 1,
               SeatType type = SeatType::STANDARD)
{
    return new Seat(roomId, id, row, column, type);
}

void checkRejectedAdd(IRepository<Seat>& repo, Seat* candidate,
                      Room* room1, Room* room2, const string& name)
{
    int count = repo.count();
    int firstCount = room1->getSeatCount();
    int secondCount = room2->getSeatCount();
    bool added = repo.add(candidate);
    check(!added && repo.count() == count && candidate->getRoom() == nullptr &&
          room1->getSeatCount() == firstCount &&
          room2->getSeatCount() == secondCount, name);
    if (!added)
    {
        delete candidate;
    }
}

void testInterfaceAndBeforeLoad()
{
    cout << "\n===== TEST INTERFACE / BEFORE LOAD =====" << endl;
    check(is_abstract<IRepository<Seat>>::value &&
          is_base_of<IRepository<Seat>, SeatRepository>::value &&
          has_virtual_destructor<IRepository<Seat>>::value &&
          !is_abstract<SeatRepository>::value, "Hop dong interface va destructor ao");
    prepare();
    RoomRepository rooms(ROOMS);
    SeatRepository concrete(SEATS, rooms);
    IRepository<Seat>& repo = concrete;
    check(!repo.isLoaded() && repo.count() == 0 &&
          repo.findById("ROM0001|A01") == nullptr &&
          repo.getAt(0) == nullptr && repo.getAt(-1) == nullptr,
          "Query truoc load");
    check(!repo.load() && !repo.isLoaded(), "Khong load khi Room chua load");
    Seat* candidate = makeSeat("A01");
    bool added = repo.add(candidate);
    check(!added && candidate->getRoom() == nullptr && repo.count() == 0,
          "Add truoc load, caller giu so huu");
    if (!added)
    {
        delete candidate;
    }
    Seat values("ROM0001", "A01", 1, 1, SeatType::VIP);
    check(!repo.update(values) && !repo.removeById("ROM0001|A01") &&
          !repo.save() && repo.count() == 0, "Update, remove, save truoc load");
    check(rooms.load() && repo.load() && repo.isLoaded() && repo.count() == 0,
          "Load file rong sau Room");
    check(!repo.load() && repo.count() == 0, "Load lap tra false");
    check(repo.save() && readTestFile(SEATS).empty(), "Save file rong");
}

void testLoadAndRollback()
{
    cout << "\n===== TEST LOAD / ROLLBACK =====" << endl;
    prepare(FIRST + SECOND + "ROM0002|A01|1|1|COUPLE\n");
    RoomRepository rooms(ROOMS);
    check(rooms.load(), "Load Room cho Seat");
    Room* room1 = rooms.findById("ROM0001");
    Room* room2 = rooms.findById("ROM0002");
    {
        SeatRepository concrete(SEATS, rooms);
        IRepository<Seat>& repo = concrete;
        check(repo.load() && repo.isLoaded() && repo.count() == 3,
              "Load hop le, cung seatId khac Room");
        Seat* first = repo.findById("ROM0001|A01");
        Seat* otherRoom = repo.findById("ROM0002|A01");
        check(first && otherRoom && first != otherRoom &&
              concrete.findById("ROM0001", "A01") == first &&
              concrete.findById("ROM0002", "A01") == otherRoom &&
              first->getRoom() == room1 && otherRoom->getRoom() == room2 &&
              room1->findSeat("A01") == first && room2->findSeat("A01") == otherRoom,
              "Lien ket hai chieu va hai cach findById");
        check(repo.getAt(0) == first && repo.getAt(1) && repo.getAt(2) == otherRoom &&
              repo.getAt(-1) == nullptr && repo.getAt(3) == nullptr &&
              repo.findById("ROM0001|X99") == nullptr,
              "GetAt bien va find khoa thieu");
        check(!repo.load() && repo.getAt(0) == first && repo.count() == 3,
              "Load lap khong thay doi du lieu");
    }
    check(room1->getSeatCount() == 0 && room2->getSeatCount() == 0,
          "Destructor go lien ket truoc khi giai phong Seat");

    const vector<pair<string, string>> invalid = {
        {"thieu truong", "ROM0001|A02|1|2\n"},
        {"thua truong", "ROM0001|A02|1|2|VIP|x\n"},
        {"roomId sai", "BAD0001|A02|1|2|VIP\n"},
        {"seatId rong", "ROM0001||1|2|VIP\n"},
        {"seatId co dau phan cach", "ROM0001|A|02|1|2|VIP\n"},
        {"row sai dinh dang", "ROM0001|A02|x|2|VIP\n"},
        {"column sai dinh dang", "ROM0001|A02|1|2x|VIP\n"},
        {"row khong duong", "ROM0001|A02|0|2|VIP\n"},
        {"column khong duong", "ROM0001|A02|1|-1|VIP\n"},
        {"row vuot Room", "ROM0001|A02|4|2|VIP\n"},
        {"column vuot Room", "ROM0001|A02|1|5|VIP\n"},
        {"SeatType sai", "ROM0001|A02|1|2|OTHER\n"},
        {"Room khong ton tai", "ROM9999|A02|1|2|VIP\n"},
        {"trung khoa ket hop", FIRST},
        {"trung vi tri", "ROM0001|A02|1|1|VIP\n"}
    };
    for (const auto& item : invalid)
    {
        writeTestFile(SEATS, FIRST + item.second);
        SeatRepository repo(SEATS, rooms);
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
              room1->getSeatCount() == 0 && room2->getSeatCount() == 0,
              "Load loi va rollback: " + item.first);
        writeTestFile(SEATS, FIRST);
        check(repo.load() && repo.count() == 1 &&
              room1->findSeat("A01") == repo.getAt(0),
              "Sua file va load lai: " + item.first);
    }
    remove(MISSING.c_str());
    SeatRepository missing(MISSING, rooms);
    check(!missing.load() && !missing.isLoaded() && missing.count() == 0,
          "File khong ton tai tra false");
}

void testRollbackKeepsExistingLinks()
{
    cout << "\n===== TEST ROLLBACK GIU LIEN KET CO SAN =====" << endl;
    prepare(FIRST + "ROM0001|A02|2|2|VIP\n");
    RoomRepository rooms(ROOMS);
    check(rooms.load(), "Load Room cho rollback");
    Room* room = rooms.findById("ROM0001");
    Seat* existing = makeSeat("Z99", "ROM0001", 3, 4);
    check(existing->linkRoom(room), "Tao lien ket co san ngoai SeatRepository");
    writeTestFile(SEATS, FIRST + "ROM0001|A02|3|4|VIP\n");
    {
        SeatRepository repo(SEATS, rooms);
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
              room->findSeat("A01") == nullptr &&
              room->findSeat("A02") == nullptr &&
              room->findSeat("Z99") == existing &&
              existing->getRoom() == room && room->getSeatCount() == 1,
              "Rollback go Seat moi, giu lien ket cu");
        writeTestFile(SEATS, FIRST + "ROM0001|A02|2|2|VIP\n");
        check(repo.load() && repo.count() == 2 && room->getSeatCount() == 3 &&
              room->findSeat("Z99") == existing,
              "Sua file va load lai sau rollback");
    }
    check(room->getSeatCount() == 1 && room->findSeat("Z99") == existing,
          "Destructor chi go Seat cua repository");
    check(room->unlinkSeat(existing), "Caller go lien ket cu");
    delete existing;
}

void testAddUpdateRemoveAndSave()
{
    cout << "\n===== TEST ADD / UPDATE / REMOVE / SAVE =====" << endl;
    prepare();
    RoomRepository rooms(ROOMS);
    check(rooms.load(), "Load Room cho CRUD");
    Room* room1 = rooms.findById("ROM0001");
    Room* room2 = rooms.findById("ROM0002");
    SeatRepository concrete(SEATS, rooms);
    IRepository<Seat>& repo = concrete;
    check(repo.load(), "Load Seat rong cho CRUD");
    Seat* first = makeSeat("A01");
    bool added = repo.add(first);
    check(added && repo.count() == 1 && repo.findById("ROM0001|A01") == first &&
          first->getRoom() == room1 && room1->findSeat("A01") == first,
          "Add thanh cong chuyen so huu va link hai chieu");
    if (!added)
    {
        delete first;
        return;
    }
    check(!repo.add(nullptr) && repo.count() == 1, "Tu choi nullptr");
    check(!repo.add(first) && repo.count() == 1 && room1->getSeatCount() == 1,
          "Tu choi con tro dang duoc quan ly");
    checkRejectedAdd(repo, makeSeat("A02", "BAD0001"), room1, room2, "Tu choi roomId sai");
    checkRejectedAdd(repo, makeSeat(""), room1, room2, "Tu choi seatId rong");
    checkRejectedAdd(repo, makeSeat("A|02"), room1, room2, "Tu choi seatId co dau phan cach");
    checkRejectedAdd(repo, makeSeat("A02", "ROM0001", 0, 2), room1, room2, "Tu choi row khong duong");
    checkRejectedAdd(repo, makeSeat("A02", "ROM0001", 1, 0), room1, room2, "Tu choi column khong duong");
    checkRejectedAdd(repo, makeSeat("A02", "ROM0001", 1, 2, static_cast<SeatType>(99)),
                     room1, room2, "Tu choi SeatType sai");
    checkRejectedAdd(repo, makeSeat("A02", "ROM9999", 1, 2), room1, room2, "Tu choi Room khong ton tai");
    checkRejectedAdd(repo, makeSeat("A02", "ROM0001", 4, 2), room1, room2, "Tu choi vi tri ngoai Room");
    checkRejectedAdd(repo, makeSeat("A01", "ROM0001", 2, 2), room1, room2, "Tu choi khoa trung");
    checkRejectedAdd(repo, makeSeat("A02", "ROM0001", 1, 1), room1, room2, "Tu choi vi tri trung");
    Seat* linked = makeSeat("A03", "ROM0002", 1, 2);
    check(linked->linkRoom(room2), "Tao Seat da lien ket truoc add");
    check(!repo.add(linked) && linked->getRoom() == room2 &&
          room2->findSeat("A03") == linked && repo.count() == 1,
          "Tu choi Seat da lien ket, khong pha lien ket cua caller");
    check(room2->unlinkSeat(linked), "Caller go lien ket sau add that bai");
    delete linked;
    Seat* otherRoom = makeSeat("A01", "ROM0002");
    added = repo.add(otherRoom);
    check(added && repo.count() == 2 && otherRoom->getRoom() == room2 &&
          room2->findSeat("A01") == otherRoom && room1->findSeat("A01") == first,
          "Cung seatId khac Room duoc them");
    if (!added)
    {
        delete otherRoom;
        return;
    }
    Seat* second = makeSeat("A02", "ROM0001", 1, 2, SeatType::VIP);
    added = repo.add(second);
    check(added && repo.count() == 3 && room1->findSeat("A02") == second,
          "Them ghe thu hai trong Room");
    if (!added)
    {
        delete second;
        return;
    }

    Seat change("ROM0001", "A01", 1, 1, SeatType::COUPLE);
    check(repo.update(change) && first->getSeatType() == SeatType::COUPLE &&
          repo.findById("ROM0001|A01") == first &&
          concrete.findById("ROM0001", "A01") == first &&
          first->getRoom() == room1 && room1->findSeat("A01") == first,
          "Update SeatType giu khoa, dia chi va lien ket");
    Seat unknown("ROM0001", "X99", 1, 1, SeatType::VIP);
    Seat badId("ROM0001", "", 1, 1, SeatType::VIP);
    Seat badType("ROM0001", "A01", 1, 1, static_cast<SeatType>(99));
    Seat moveRow("ROM0001", "A01", 2, 1, SeatType::VIP);
    Seat moveColumn("ROM0001", "A01", 1, 2, SeatType::VIP);
    check(!repo.update(unknown) && !repo.update(badId) &&
          !repo.update(badType) && !repo.update(moveRow) &&
          !repo.update(moveColumn) && first->getSeatType() == SeatType::COUPLE &&
          first->getRow() == 1 && first->getColumn() == 1 &&
          first->getRoom() == room1 && repo.count() == 3,
          "Update that bai giu nguyen du lieu");
    check(repo.save(), "Save qua IRepository");
    check(readTestFile(SEATS) ==
          "ROM0001|A01|1|1|COUPLE\n"
          "ROM0002|A01|1|1|STANDARD\n"
          "ROM0001|A02|1|2|VIP\n",
          "TXT dung schema, khoa va thu tu");
    check(repo.removeById("ROM0001|A02") && repo.count() == 2 &&
          room1->findSeat("A02") == nullptr && room1->getSeatCount() == 1,
          "Remove bang khoa ket hop");
    check(concrete.removeById("ROM0002", "A01") && repo.count() == 1 &&
          room2->findSeat("A01") == nullptr && room2->getSeatCount() == 0 &&
          room1->findSeat("A01") == first,
          "Remove bang hai tham so, giu Seat khac Room");
    check(!repo.removeById("ROM0002|A01") &&
          !concrete.removeById("ROM0001", "X99") && repo.count() == 1 &&
          room1->findSeat("A01") == first,
          "Remove Seat khong ton tai khong doi du lieu");
    check(repo.save() && readTestFile(SEATS) == "ROM0001|A01|1|1|COUPLE\n",
          "Save sau remove");
}

void testReloadSaved()
{
    cout << "\n===== TEST RELOAD SAVED =====" << endl;
    RoomRepository rooms(ROOMS);
    check(rooms.load(), "Load lai Room");
    SeatRepository repo(SEATS, rooms);
    Seat* seat = nullptr;
    bool loaded = repo.load();
    if (loaded)
    {
        seat = repo.findById("ROM0001", "A01");
    }
    check(loaded && repo.isLoaded() && repo.count() == 1 && seat &&
          seat->getSeatType() == SeatType::COUPLE &&
          seat->getRoom() == rooms.findById("ROM0001") &&
          rooms.findById("ROM0001")->findSeat("A01") == seat,
          "Load lai du lieu da save va lien ket hai chieu");
}

int main()
{
    try
    {
        testInterfaceAndBeforeLoad();
        testLoadAndRollback();
        testRollbackKeepsExistingLinks();
        testAddUpdateRemoveAndSave();
        testReloadSaved();
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }
    remove(ROOMS.c_str());
    remove(SEATS.c_str());
    remove(MISSING.c_str());
    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
