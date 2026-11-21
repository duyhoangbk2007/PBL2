#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <vector>

#include "../src/business/MovieService.h"
#include "../src/business/RoomService.h"
#include "../src/business/ShowtimeService.h"
#include "../src/business/StaffService.h"
#include "../src/business/MemberService.h"
#include "../src/business/AuthService.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/utils/IdGenerator.h"

using namespace std;

static int passed = 0;
static int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (condition) ++passed;
    else ++failed;
}

void writeFile(const string& path, const string& content)
{
    ofstream out(path, ios::trunc);
    if (!out || !(out << content))
        throw runtime_error("Khong ghi duoc file test: " + path);
}

bool contains(const string& path, const string& text)
{
    ifstream in(path);
    string line;
    while (getline(in, line))
        if (line.find(text) != string::npos) return true;
    return false;
}

void checkIdGenerator()
{
    check(IdGenerator::generateNextId("MOV", {}) == "MOV0001",
          "IdGenerator: danh sach rong");
    check(IdGenerator::generateNextId("MOV", {"MOV0001", "MOV0003"}) == "MOV0004",
          "IdGenerator: lay ID lon nhat + 1");
    check(IdGenerator::generateNextId("ROM", {"ROM0002"}) == "ROM0003",
          "IdGenerator: ROM");
    bool threw = false;
    try { IdGenerator::generateNextId("MOV", {"MOV9999"}); }
    catch (const exception&) { threw = true; }
    check(threw, "IdGenerator: het ID");
    threw = false;
    try { IdGenerator::generateNextId("MOV", {"ROM0001"}); }
    catch (const exception&) { threw = true; }
    check(threw, "IdGenerator: sai tien to");
}

int main()
{
    const string prefix = "test_auto_id_";
    const string userPath = prefix + "users.txt";
    const string permissionPath = prefix + "permissions.txt";
    const string moviePath = prefix + "movies.txt";
    const string roomPath = prefix + "rooms.txt";
    const string showtimePath = prefix + "showtimes.txt";
    const string memberPath = prefix + "members.txt";
    const vector<string> paths = {
        userPath, permissionPath, moviePath, roomPath, showtimePath, memberPath
    };

    try
    {
        // Du lieu test rieng, KHONG dung file TXT that.
        writeFile(userPath, "USR0001|admin|123456|Admin Test|ADMIN|1\n");
        writeFile(permissionPath,
                  "ADMIN|MOVIE_ADD\n"
                  "ADMIN|MOVIE_DELETE\n"
                  "ADMIN|MOVIE_VIEW\n"
                  "ADMIN|ROOM_ADD\n"
                  "ADMIN|ROOM_VIEW\n"
                  "ADMIN|SHOWTIME_ADD\n"
                  "ADMIN|SHOWTIME_VIEW\n"
                  "ADMIN|STAFF_ADD\n"
                  "ADMIN|STAFF_VIEW\n"
                  "ADMIN|MEMBER_ADD\n"
                  "ADMIN|MEMBER_VIEW\n");
        writeFile(moviePath, "");
        writeFile(roomPath, "");
        writeFile(showtimePath, "");
        writeFile(memberPath, "");

        checkIdGenerator();

        {
            UserRepository userRepo(userPath);
            RolePermissionRepository permissionRepo(permissionPath);
            MovieRepository movieRepo(moviePath);
            RoomRepository roomRepo(roomPath);
            ShowtimeRepository showtimeRepo(showtimePath, movieRepo, roomRepo);
            MemberRepository memberRepo(memberPath);

            check(userRepo.load(), "Load User");
            check(permissionRepo.load(), "Load Permissions");
            check(movieRepo.load(), "Load Movie");
            check(roomRepo.load(), "Load Room");
            check(showtimeRepo.load(), "Load Showtime");
            check(memberRepo.load(), "Load Member");

            AuthService auth(userRepo, permissionRepo);
            check(auth.login("admin", "123456"), "Login Admin");

            MovieService movies(movieRepo, auth);
            RoomService rooms(roomRepo, auth);
            ShowtimeService showtimes(showtimeRepo, movieRepo, roomRepo, auth);
            StaffService staff(userRepo, auth);
            MemberService members(memberRepo, auth);

            Movie movie("", "Test Movie", "Action", 120, 13,
                        "2098-01-01", "2099-12-31", true);
            check(movies.addMovie(&movie), "Movie: add 1");
            check(movieRepo.findById("MOV0001") != nullptr, "Movie: MOV0001");
            check(movies.addMovie(&movie), "Movie: add 2");
            check(movieRepo.findById("MOV0002") != nullptr, "Movie: MOV0002");
            check(movie.getMovieId().empty(), "Movie: input khong bi doi ID");
            check(movies.removeMovie("MOV0002"), "Movie: xoa ID lon nhat");
            check(movies.addMovie(&movie), "Movie: add lai");
            check(movieRepo.findById("MOV0002") != nullptr, "Movie: tai su dung MOV0002");
            check(!movies.addMovie(nullptr), "Movie: nullptr");
            Movie badMovie("", "", "Action", 120, 13,
                           "2098-01-01", "2099-12-31", true);
            check(!movies.addMovie(&badMovie), "Movie: du lieu sai");
            check(movieRepo.count() == 2, "Movie: count sau add loi");

            Room room("", "Room Test", 5, 6, true);
            check(rooms.addRoom(&room), "Room: add 1");
            check(roomRepo.findById("ROM0001") != nullptr, "Room: ROM0001");
            check(rooms.addRoom(&room), "Room: add 2");
            check(roomRepo.findById("ROM0002") != nullptr, "Room: ROM0002");
            check(room.getRoomId().empty(), "Room: input khong bi doi ID");
            Room badRoom("", "Bad Room", 0, 6, true);
            check(!rooms.addRoom(&badRoom), "Room: kich thuoc sai");
            check(!rooms.addRoom(nullptr), "Room: nullptr");
            check(roomRepo.count() == 2, "Room: count sau add loi");

            Showtime showtime("", "MOV0001", "ROM0001",
                              "2099-01-01T10:00:00", "2099-01-01T12:00:00",
                              ShowtimeStatus::SCHEDULED);
            check(showtimes.addShowtime(&showtime), "Showtime: add 1");
            check(showtimeRepo.findById("SHW0001") != nullptr, "Showtime: SHW0001");
            check(showtime.getShowtimeId().empty(), "Showtime: input khong bi doi ID");
            check(!showtimes.addShowtime(&showtime), "Showtime: trung lich");
            Showtime badShowtime("", "MOV0001", "ROM0002",
                                 "2099-01-01T10:00:00", "2099-01-01T11:00:00",
                                 ShowtimeStatus::SCHEDULED);
            check(!showtimes.addShowtime(&badShowtime), "Showtime: sai thoi luong");
            check(!showtimes.addShowtime(nullptr), "Showtime: nullptr");
            check(showtimeRepo.count() == 1, "Showtime: count sau add loi");
            const Showtime* linked = showtimeRepo.findById("SHW0001");
            check(linked != nullptr && linked->getMovie() != nullptr &&
                  linked->getRoom() != nullptr, "Showtime: lien ket Movie/Room");

            User staffInput("", "staff_test", "123456", "Staff Test",
                            UserRole::STAFF, true);
            check(staff.addStaff(&staffInput), "Staff: add");
            check(userRepo.findById("USR0002") != nullptr, "Staff: USR0002 sau Admin");
            check(staffInput.getUserId().empty(), "Staff: input khong bi doi ID");
            check(!staff.addStaff(&staffInput), "Staff: trung username");
            User adminInput("", "another_admin", "123456", "Another Admin",
                            UserRole::ADMIN, true);
            check(!staff.addStaff(&adminInput), "Staff: khong them Admin");
            check(!staff.addStaff(nullptr), "Staff: nullptr");
            check(userRepo.count() == 2, "Staff: count sau add loi");

            Member member("", "Member Test", "0912345678", 0);
            check(members.addMember(&member), "Member: add");
            check(memberRepo.findById("MEM0001") != nullptr, "Member: MEM0001");
            check(member.getMemberId().empty(), "Member: input khong bi doi ID");
            check(!members.addMember(&member), "Member: trung phone");
            Member badMember("", "Member Bad", "0987654321", 10);
            check(!members.addMember(&badMember), "Member: diem ban dau khac 0");
            check(!members.addMember(nullptr), "Member: nullptr");
            check(memberRepo.count() == 1, "Member: count sau add loi");

            // Kiem tra save rieng; add khong tu dong ghi TXT.
            check(!contains(moviePath, "MOV0001"), "Movie: chua save thi TXT chua doi");
            check(movies.save(), "Movie: save");
            check(rooms.save(), "Room: save");
            check(showtimes.save(), "Showtime: save");
            check(staff.save(), "Staff: save");
            check(members.save(), "Member: save");
            check(contains(moviePath, "MOV0002|"), "Movie: TXT co MOV0002");
            check(contains(roomPath, "ROM0001|"), "Room: TXT co ROM0001");
            check(contains(showtimePath, "SHW0001|"), "Showtime: TXT co SHW0001");
            check(contains(userPath, "USR0002|"), "Staff: TXT co USR0002");
            check(contains(memberPath, "MEM0001|"), "Member: TXT co MEM0001");

            auth.logout();
            bool denied = false;
            try { movies.addMovie(&movie); }
            catch (const exception&) { denied = true; }
            check(denied, "Auth: chua login khong duoc add");
        }

        // Tao Repository moi de kiem tra du lieu TXT doc lai duoc.
        {
            UserRepository users(userPath);
            MovieRepository movies(moviePath);
            RoomRepository rooms(roomPath);
            ShowtimeRepository shows(showtimePath, movies, rooms);
            MemberRepository members(memberPath);

            check(users.load(), "Reload User");
            check(movies.load(), "Reload Movie");
            check(rooms.load(), "Reload Room");
            check(shows.load(), "Reload Showtime");
            check(members.load(), "Reload Member");
            check(users.findById("USR0002") != nullptr, "Reload: Staff ton tai");
            check(movies.findById("MOV0002") != nullptr, "Reload: Movie ton tai");
            check(rooms.findById("ROM0001") != nullptr, "Reload: Room ton tai");
            check(shows.findById("SHW0001") != nullptr, "Reload: Showtime ton tai");
            check(members.findById("MEM0001") != nullptr, "Reload: Member ton tai");
        }
    }
    catch (const exception& e)
    {
        ++failed;
        cerr << "[ERROR] " << e.what() << '\n';
    }

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "Total: " << passed + failed << '\n';

    // Chi xoa file test neu tat ca PASS, giu lai de debug neu FAIL.
    if (failed == 0)
        for (const string& path : paths) remove(path.c_str());

    return failed == 0 ? 0 : 1;
}
