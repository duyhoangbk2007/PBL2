#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "../src/data_access/IRepository.h"
#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/ShowtimeRepository.h"

using namespace std;

int passed = 0;
int failed = 0;
const string MOVIES = "test_showtime_repository_movies.txt";
const string ROOMS = "test_showtime_repository_rooms.txt";
const string SHOWS = "test_showtime_repository_showtimes.txt";
const string MISSING = "test_showtime_repository_missing.txt";
const string MOVIE_DATA =
    "MOV0001|Phim 1|Action|120|13|2026-01-01|2028-12-31|1\n"
    "MOV0002|Phim 2|Comedy|90|0|2026-01-01|2028-12-31|1\n"
    "MOV0003|Phim cu|Drama|100|0|2026-01-01|2028-12-31|0\n";
const string ROOM_DATA =
    "ROM0001|Phong 1|8|12|1\n"
    "ROM0002|Phong 2|8|12|1\n"
    "ROM0003|Phong cu|8|12|0\n";
const string FIRST = "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n";
const string SECOND = "SHW0002|MOV0002|ROM0001|2027-01-10T12:15:00|2027-01-10T13:45:00|SCHEDULED\n";

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
        if (ch != '\r') normalized += ch;
    }
    return normalized;
}

void prepare(const string& shows = "")
{
    writeTestFile(MOVIES, MOVIE_DATA);
    writeTestFile(ROOMS, ROOM_DATA);
    writeTestFile(SHOWS, shows);
}

Showtime* makeShow(const string& id, const string& movie = "MOV0001",
                   const string& room = "ROM0001",
                   const string& start = "2027-01-10T10:00:00",
                   const string& end = "2027-01-10T12:00:00",
                   ShowtimeStatus status = ShowtimeStatus::SCHEDULED)
{
    return new Showtime(id, movie, room, start, end, status);
}

void checkRejectedAdd(IRepository<Showtime>& repo, Showtime* candidate,
                      Movie* movie, Room* room, const string& name)
{
    int count = repo.count();
    int movieCount = movie->getShowtimeCount();
    int roomCount = room->getShowtimeCount();
    bool added = repo.add(candidate);
    check(!added && repo.count() == count && candidate->getMovie() == nullptr &&
          candidate->getRoom() == nullptr && movie->getShowtimeCount() == movieCount &&
          room->getShowtimeCount() == roomCount, name);
    if (!added)
    {
        delete candidate;
    }
}

void testInterfaceAndBeforeLoad()
{
    cout << "\n===== TEST INTERFACE / BEFORE LOAD =====" << endl;
    check(is_abstract<IRepository<Showtime>>::value &&
          is_base_of<IRepository<Showtime>, ShowtimeRepository>::value &&
          has_virtual_destructor<IRepository<Showtime>>::value &&
          !is_abstract<ShowtimeRepository>::value, "Hop dong interface va destructor ao");
    prepare();
    MovieRepository movies(MOVIES);
    RoomRepository rooms(ROOMS);
    ShowtimeRepository concrete(SHOWS, movies, rooms);
    IRepository<Showtime>& repo = concrete;
    check(!repo.isLoaded() && repo.count() == 0 && repo.findById("SHW0001") == nullptr &&
          repo.getAt(0) == nullptr, "Query truoc load");
    check(!repo.load() && !repo.isLoaded(), "Khong load khi Movie va Room chua load");
    check(movies.load() && !repo.load() && !repo.isLoaded(), "Khong load khi Room chua load");
    check(rooms.load(), "Load dependency dung thu tu");
    Showtime* candidate = makeShow("SHW0001");
    bool added = repo.add(candidate);
    check(!added && candidate->getMovie() == nullptr && candidate->getRoom() == nullptr,
          "Add truoc load tra false, caller giu so huu");
    if (!added) delete candidate;
    Showtime values("SHW0001", "MOV0001", "ROM0001", "2027-01-10T10:00:00",
                    "2027-01-10T12:00:00", ShowtimeStatus::SCHEDULED);
    check(!repo.save() && !repo.update(values) && !repo.removeById("SHW0001") &&
          repo.count() == 0, "Save, update, remove truoc load");
    check(repo.load() && repo.isLoaded() && repo.count() == 0, "Load file rong");
    check(!repo.load() && repo.count() == 0, "Load lan hai tra false");
    check(repo.save() && readTestFile(SHOWS).empty(), "Save file rong");
}

void testLoadAndRollback()
{
    cout << "\n===== TEST LOAD / ROLLBACK =====" << endl;
    prepare(FIRST + SECOND);
    MovieRepository movies(MOVIES);
    RoomRepository rooms(ROOMS);
    check(movies.load() && rooms.load(), "Load Movie va Room");
    {
        ShowtimeRepository concrete(SHOWS, movies, rooms);
        IRepository<Showtime>& repo = concrete;
        check(repo.load() && repo.isLoaded() && repo.count() == 2, "Load hai suat hop le");
        Showtime* first = repo.findById("SHW0001");
        Showtime* second = repo.findById("SHW0002");
        check(first && second && repo.getAt(0) == first && repo.getAt(1) == second &&
              repo.getAt(-1) == nullptr && repo.getAt(2) == nullptr &&
              repo.findById("SHW9999") == nullptr, "Find, getAt, count qua interface");
        check(first && first->getMovie() == movies.findById("MOV0001") &&
              first->getRoom() == rooms.findById("ROM0001") &&
              movies.findById("MOV0001")->findShowtime("SHW0001") == first &&
              rooms.findById("ROM0001")->findShowtime("SHW0001") == first,
              "Load lien ket hai chieu");
        check(!repo.load() && repo.findById("SHW0001") == first && repo.count() == 2,
              "Load lan hai khong doi con tro va du lieu");
    }
    check(movies.findById("MOV0001")->getShowtimeCount() == 0 &&
          rooms.findById("ROM0001")->getShowtimeCount() == 0,
          "Destructor go lien ket truoc khi huy Showtime");

    const vector<pair<string, string>> invalid = {
        {"thieu truong", "SHW0002|MOV0001|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00\n"},
        {"thua truong", "SHW0002|MOV0001|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED|x\n"},
        {"ID suat sai", "BAD0002|MOV0001|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED\n"},
        {"ID Movie sai", "SHW0002|BAD0001|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED\n"},
        {"ID Room sai", "SHW0002|MOV0001|BAD0001|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED\n"},
        {"thoi gian sai", "SHW0002|MOV0001|ROM0001|2027-01-10 13:00:00|2027-01-10T14:00:00|SCHEDULED\n"},
        {"ngay khong hop le", "SHW0002|MOV0001|ROM0001|2027-02-30T13:00:00|2027-02-30T14:00:00|SCHEDULED\n"},
        {"end truoc start", "SHW0002|MOV0001|ROM0001|2027-01-10T14:00:00|2027-01-10T13:00:00|SCHEDULED\n"},
        {"status sai", "SHW0002|MOV0001|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00|DONE\n"},
        {"ID trung", FIRST},
        {"lich trung", "SHW0002|MOV0002|ROM0001|2027-01-10T12:14:00|2027-01-10T13:44:00|SCHEDULED\n"},
        {"Movie thieu", "SHW0002|MOV9999|ROM0001|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED\n"},
        {"Room thieu", "SHW0002|MOV0001|ROM9999|2027-01-10T13:00:00|2027-01-10T14:00:00|SCHEDULED\n"}
    };
    for (const auto& item : invalid)
    {
        writeTestFile(SHOWS, FIRST + item.second);
        ShowtimeRepository concrete(SHOWS, movies, rooms);
        IRepository<Showtime>& repo = concrete;
        bool threw = false;
        try { repo.load(); }
        catch (const runtime_error&) { threw = true; }
        check(threw && !repo.isLoaded() && repo.count() == 0 &&
              repo.findById("SHW0001") == nullptr &&
              movies.findById("MOV0001")->getShowtimeCount() == 0 &&
              rooms.findById("ROM0001")->getShowtimeCount() == 0,
              "Load loi va rollback: " + item.first);
        writeTestFile(SHOWS, FIRST);
        check(repo.load() && repo.count() == 1 &&
              movies.findById("MOV0001")->findShowtime("SHW0001") == repo.getAt(0),
              "Sua file va load lai: " + item.first);
    }
    writeTestFile(SHOWS,
        "SHW0001|MOV0003|ROM0003|2027-01-10T10:00:00|2027-01-10T12:00:00|CANCELLED\n");
    {
        ShowtimeRepository repo(SHOWS, movies, rooms);
        check(repo.load() && repo.count() == 1 &&
              repo.getAt(0)->getMovie() == movies.findById("MOV0003") &&
              repo.getAt(0)->getRoom() == rooms.findById("ROM0003"),
              "Load suat lich su voi Movie va Room inactive");
    }
    remove(MISSING.c_str());
    ShowtimeRepository missing(MISSING, movies, rooms);
    check(!missing.load() && !missing.isLoaded() && missing.count() == 0,
          "File khong ton tai tra false");
}

void testCrudAndSave()
{
    cout << "\n===== TEST ADD / SCHEDULE / UPDATE / REMOVE / SAVE =====" << endl;
    prepare();
    MovieRepository movies(MOVIES);
    RoomRepository rooms(ROOMS);
    check(movies.load() && rooms.load(), "Load dependency cho CRUD");
    ShowtimeRepository concrete(SHOWS, movies, rooms);
    IRepository<Showtime>& repo = concrete;
    check(repo.load(), "Load Showtime rong cho CRUD");
    Movie* movie1 = movies.findById("MOV0001");
    Room* room1 = rooms.findById("ROM0001");
    Showtime* first = makeShow("SHW0001");
    bool added = repo.add(first);
    check(added && repo.findById("SHW0001") == first && repo.count() == 1 &&
          first->getMovie() == movie1 && first->getRoom() == room1 &&
          movie1->findShowtime("SHW0001") == first && room1->findShowtime("SHW0001") == first,
          "Add thanh cong chuyen so huu va link hai chieu");
    if (!added) delete first;
    check(!repo.add(nullptr) && repo.count() == 1, "Tu choi nullptr");
    checkRejectedAdd(repo, makeShow("SHW0001"), movie1, room1, "Tu choi ID trung");
    checkRejectedAdd(repo, makeShow("BAD0002"), movie1, room1, "Tu choi ID sai");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV9999"), movie1, room1, "Tu choi Movie thieu");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM9999"), movie1, room1, "Tu choi Room thieu");
    checkRejectedAdd(repo, makeShow("SHW0002", "BAD0001"), movie1, room1, "Tu choi Movie ID sai");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "BAD0001"), movie1, room1, "Tu choi Room ID sai");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0003"), movie1, room1, "Tu choi Movie inactive SCHEDULED");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0003"), movie1, room1, "Tu choi Room inactive SCHEDULED");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0001", "bad"), movie1, room1, "Tu choi start sai");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10T12:00:00", "bad"), movie1, room1, "Tu choi end sai");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10T12:00:00", "2027-01-10T12:00:00"), movie1, room1, "Tu choi end bang start");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10T12:00:00", "2027-01-10T11:00:00"), movie1, room1, "Tu choi end truoc start");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10T13:00:00", "2027-01-10T14:00:00", static_cast<ShowtimeStatus>(99)), movie1, room1, "Tu choi enum sai");
    Showtime* linked = makeShow("SHW0002", "MOV0002", "ROM0002");
    check(linked->linkMovie(movies.findById("MOV0002")), "Tao lien ket truoc add");
    check(!repo.add(linked) && linked->getMovie() == movies.findById("MOV0002") &&
          linked->getRoom() == nullptr && repo.count() == 1,
          "Tu choi Showtime da link, khong pha link cua caller");
    check(movies.findById("MOV0002")->unlinkShowtime(linked), "Caller go link sau add that bai");
    delete linked;
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0002", "ROM0001", "2027-01-10T11:00:00", "2027-01-10T12:30:00"), movie1, room1, "Tu choi trung lich cung phong");
    checkRejectedAdd(repo, makeShow("SHW0002", "MOV0002", "ROM0001", "2027-01-10T12:14:00", "2027-01-10T13:44:00"), movie1, room1, "Nghi 14 phut bi tu choi");
    Showtime* second = makeShow("SHW0002", "MOV0002", "ROM0001", "2027-01-10T12:15:00", "2027-01-10T13:45:00");
    added = repo.add(second);
    check(added && room1->findShowtime("SHW0002") == second, "Nghi dung 15 phut duoc chap nhan");
    if (!added) delete second;
    Showtime* otherRoom = makeShow("SHW0003", "MOV0002", "ROM0002");
    added = repo.add(otherRoom);
    check(added && otherRoom->getRoom() == rooms.findById("ROM0002"), "Cung gio khac phong");
    if (!added) delete otherRoom;
    Showtime* cancelled = makeShow("SHW0004", "MOV0003", "ROM0003", "2027-01-10T10:00:00", "2027-01-10T12:00:00", ShowtimeStatus::CANCELLED);
    added = repo.add(cancelled);
    check(added && cancelled->getMovie() == movies.findById("MOV0003"), "CANCELLED cho phep tham chieu inactive");
    if (!added) delete cancelled;
    Showtime* cancelledOverlap = makeShow("SHW0005", "MOV0001", "ROM0001", "2027-01-10T10:00:00", "2027-01-10T12:00:00", ShowtimeStatus::CANCELLED);
    added = repo.add(cancelledOverlap);
    check(added && repo.count() == 5, "CANCELLED bo qua trung lich");
    if (!added) delete cancelledOverlap;
    Showtime* afterCancelled = makeShow("SHW0006", "MOV0002", "ROM0001", "2027-01-10T08:00:00", "2027-01-10T09:45:00");
    added = repo.add(afterCancelled);
    check(added, "SCHEDULED bo qua suat CANCELLED");
    if (!added) delete afterCancelled;

    Showtime same("SHW0002", "MOV0002", "ROM0001", "2027-01-10T12:15:00", "2027-01-10T13:45:00", ShowtimeStatus::SCHEDULED);
    check(repo.update(same) && repo.findById("SHW0002") == second, "Update bo qua chinh no");
    Showtime moveTime("SHW0002", "MOV0002", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:30:00", ShowtimeStatus::SCHEDULED);
    check(repo.update(moveTime) && second->getStartAt() == "2027-01-10T14:00:00", "Update thoi gian");
    Showtime moveMovie("SHW0002", "MOV0001", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:30:00", ShowtimeStatus::SCHEDULED);
    check(repo.update(moveMovie) && second->getMovie() == movie1 &&
          movies.findById("MOV0002")->findShowtime("SHW0002") == nullptr &&
          movie1->findShowtime("SHW0002") == second, "Update Movie va link cu/moi");
    Showtime moveRoom("SHW0002", "MOV0001", "ROM0002", "2027-01-10T14:00:00", "2027-01-10T15:30:00", ShowtimeStatus::SCHEDULED);
    check(repo.update(moveRoom) && second->getRoom() == rooms.findById("ROM0002") &&
          room1->findShowtime("SHW0002") == nullptr, "Update Room va link cu/moi");
    Showtime moveBoth("SHW0002", "MOV0002", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:30:00", ShowtimeStatus::CANCELLED);
    check(repo.update(moveBoth) && second->getMovie() == movies.findById("MOV0002") &&
          second->getRoom() == room1 && second->getStatus() == ShowtimeStatus::CANCELLED &&
          rooms.findById("ROM0002")->findShowtime("SHW0002") == nullptr &&
          movie1->findShowtime("SHW0002") == nullptr,
          "Update dong thoi Movie, Room, status va link hai chieu");
    check(repo.findById("SHW0002") == second && second->getShowtimeId() == "SHW0002",
          "Update giu ID va dia chi");
    Showtime badId("SHW9999", "MOV0001", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime badData("SHW0002", "MOV0001", "ROM0001", "bad", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime badMovie("SHW0002", "MOV9999", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime badRoom("SHW0002", "MOV0001", "ROM9999", "2027-01-10T14:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime inactiveMovie("SHW0002", "MOV0003", "ROM0001", "2027-01-10T14:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime inactiveRoom("SHW0002", "MOV0001", "ROM0003", "2027-01-10T14:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    Showtime conflict("SHW0002", "MOV0002", "ROM0001", "2027-01-10T10:00:00", "2027-01-10T11:00:00", ShowtimeStatus::SCHEDULED);
    check(!repo.update(badId) && !repo.update(badData) && !repo.update(badMovie) &&
          !repo.update(badRoom) && !repo.update(inactiveMovie) && !repo.update(inactiveRoom) &&
          !repo.update(conflict) && repo.findById("SHW0002") == second &&
          second->getMovieId() == "MOV0002" && second->getRoomId() == "ROM0001" &&
          second->getStartAt() == "2027-01-10T14:00:00" &&
          second->getStatus() == ShowtimeStatus::CANCELLED &&
          movies.findById("MOV0002")->findShowtime("SHW0002") == second &&
          room1->findShowtime("SHW0002") == second && repo.count() == 6,
          "Update validation va conflict tra false, giu du lieu/link");
    check(repo.save(), "Save qua IRepository");
    check(readTestFile(SHOWS) == FIRST +
          "SHW0002|MOV0002|ROM0001|2027-01-10T14:00:00|2027-01-10T15:30:00|CANCELLED\n"
          "SHW0003|MOV0002|ROM0002|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n"
          "SHW0004|MOV0003|ROM0003|2027-01-10T10:00:00|2027-01-10T12:00:00|CANCELLED\n"
          "SHW0005|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|CANCELLED\n"
          "SHW0006|MOV0002|ROM0001|2027-01-10T08:00:00|2027-01-10T09:45:00|SCHEDULED\n",
          "Noi dung TXT dung schema va thu tu");
    check(repo.removeById("SHW0002") && repo.findById("SHW0002") == nullptr &&
          movies.findById("MOV0002")->findShowtime("SHW0002") == nullptr &&
          room1->findShowtime("SHW0002") == nullptr && repo.count() == 5,
          "Remove xoa doi tuong va go hai lien ket");
    check(!repo.removeById("SHW0002") && !repo.removeById("SHW9999") &&
          repo.findById("SHW0001") == first && repo.count() == 5,
          "Remove ID khong ton tai giu du lieu con lai");
    check(repo.save(), "Save sau remove");
}

void testReloadSaved()
{
    cout << "\n===== TEST RELOAD SAVED =====" << endl;
    MovieRepository movies(MOVIES);
    RoomRepository rooms(ROOMS);
    check(movies.load() && rooms.load(), "Load lai dependency");
    ShowtimeRepository repo(SHOWS, movies, rooms);
    check(repo.load() && repo.count() == 5 && repo.findById("SHW0002") == nullptr &&
          repo.findById("SHW0001") != nullptr &&
          repo.findById("SHW0004")->getStatus() == ShowtimeStatus::CANCELLED,
          "Load lai du lieu da save, khong co suat da xoa");
}

int main()
{
    try
    {
        testInterfaceAndBeforeLoad();
        testLoadAndRollback();
        testCrudAndSave();
        testReloadSaved();
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }
    remove(MOVIES.c_str());
    remove(ROOMS.c_str());
    remove(SHOWS.c_str());
    remove(MISSING.c_str());
    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
