#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "../src/business/MovieService.h"
#include "../src/business/RoomService.h"
#include "../src/business/ShowtimeService.h"
#include "../src/business/StaffService.h"
#include "../src/business/MemberService.h"
#include "../src/business/AuthService.h"
#include "../src/data_access/RolePermissionRepository.h"

using namespace std;

static int passed = 0;
static int failed = 0;

void check(bool condition, const string& name)
{
    if (condition) { cout << "[PASS] " << name << '\n'; passed++; }
    else { cout << "[FAIL] " << name << '\n'; failed++; }
}

void writeFile(const string& path, const string& content)
{
    ofstream out(path);
    if (!out) throw runtime_error("Khong tao duoc file test: " + path);
    out << content;
}

int main()
{
    try
    {
        const string prefix = "test_auto_id_";
        const string userPath = prefix + "users.txt";
        const string permissionPath = prefix + "permissions.txt";
        const string moviePath = prefix + "movies.txt";
        const string roomPath = prefix + "rooms.txt";
        const string showtimePath = prefix + "showtimes.txt";
        const string memberPath = prefix + "members.txt";

        writeFile(userPath, "USR0001|admin|123456|Admin Test|ADMIN|1\n");
        writeFile(permissionPath,
                  "ADMIN|MOVIE_ADD\n"
                  "ADMIN|ROOM_ADD\n"
                  "ADMIN|SHOWTIME_ADD\n"
                  "ADMIN|STAFF_ADD\n"
                  "ADMIN|MEMBER_ADD\n");
        writeFile(moviePath, "");
        writeFile(roomPath, "");
        writeFile(showtimePath, "");
        writeFile(memberPath, "");

        UserRepository userRepo(userPath);
        RolePermissionRepository permissionRepo(permissionPath);
        MovieRepository movieRepo(moviePath);
        RoomRepository roomRepo(roomPath);
        ShowtimeRepository showtimeRepo(showtimePath, movieRepo, roomRepo);
        MemberRepository memberRepo(memberPath);

        check(userRepo.load(), "Load User");
        check(permissionRepo.load(), "Load permissions");
        check(movieRepo.load(), "Load Movie");
        check(roomRepo.load(), "Load Room");
        check(showtimeRepo.load(), "Load Showtime");
        check(memberRepo.load(), "Load Member");

        AuthService auth(userRepo, permissionRepo);
        check(auth.login("admin", "123456"), "Dang nhap Admin");

        MovieService movieService(movieRepo, auth);
        RoomService roomService(roomRepo, auth);
        ShowtimeService showtimeService(showtimeRepo, movieRepo, roomRepo, auth);
        StaffService staffService(userRepo, auth);
        MemberService memberService(memberRepo, auth);

        Movie movieInput("", "Test Movie", "Action", 120, 13,
                         "2098-01-01", "2099-12-31", true);
        check(movieService.addMovie(&movieInput), "Them Movie dau tien");
        check(movieRepo.findById("MOV0001") != nullptr, "Movie ID = MOV0001");
        check(movieInput.getMovieId().empty(), "Movie dau vao khong bi doi ID");
        check(movieService.addMovie(&movieInput), "Them Movie thu hai");
        check(movieRepo.findById("MOV0002") != nullptr, "Movie ID = MOV0002");
        check(movieService.removeMovie("MOV0002"), "Xoa Movie ID lon nhat");
        check(movieService.addMovie(&movieInput), "Them lai sau khi xoa");
        check(movieRepo.findById("MOV0002") != nullptr, "Tai su dung MOV0002");
        check(!movieService.addMovie(nullptr), "Movie nullptr bi tu choi");
        Movie badMovie("", "", "Action", 120, 13, "2098-01-01", "2099-12-31", true);
        check(!movieService.addMovie(&badMovie), "Movie khong hop le bi tu choi");
        check(movieRepo.count() == 2, "Movie count khong doi khi add loi");

        Room roomInput("", "Room Test", 5, 6, true);
        check(roomService.addRoom(&roomInput), "Them Room dau tien");
        check(roomRepo.findById("ROM0001") != nullptr, "Room ID = ROM0001");
        check(roomInput.getRoomId().empty(), "Room dau vao khong bi doi ID");
        Room badRoom("", "Room Bad", 0, 6, true);
        check(!roomService.addRoom(&badRoom), "Room sai kich thuoc bi tu choi");
        check(!roomService.addRoom(nullptr), "Room nullptr bi tu choi");

        Showtime showtimeInput("", "MOV0001", "ROM0001",
                              "2099-01-01 10:00:00", "2099-01-01 12:00:00",
                              ShowtimeStatus::SCHEDULED);
        check(showtimeService.addShowtime(&showtimeInput), "Them Showtime dau tien");
        check(showtimeRepo.findById("SHW0001") != nullptr, "Showtime ID = SHW0001");
        check(showtimeInput.getShowtimeId().empty(), "Showtime dau vao khong bi doi ID");
        check(!showtimeService.addShowtime(&showtimeInput), "Trung lich Showtime bi tu choi");
        check(!showtimeService.addShowtime(nullptr), "Showtime nullptr bi tu choi");

        User staffInput("", "staff_test", "123456", "Staff Test", UserRole::STAFF, true);
        check(staffService.addStaff(&staffInput), "Them Staff dau tien");
        check(userRepo.findById("USR0002") != nullptr, "Staff ID = USR0002 sau Admin");
        check(staffInput.getUserId().empty(), "Staff dau vao khong bi doi ID");
        check(!staffService.addStaff(&staffInput), "Username trung bi tu choi");
        User adminInput("", "another_admin", "123456", "Another Admin", UserRole::ADMIN, true);
        check(!staffService.addStaff(&adminInput), "Khong them Admin qua StaffService");

        Member memberInput("", "Member Test", "0912345678", 0);
        check(memberService.addMember(&memberInput), "Them Member dau tien");
        check(memberRepo.findById("MEM0001") != nullptr, "Member ID = MEM0001");
        check(memberInput.getMemberId().empty(), "Member dau vao khong bi doi ID");
        check(!memberService.addMember(&memberInput), "So dien thoai trung bi tu choi");
        Member badMember("", "Member Bad", "0987654321", 10);
        check(!memberService.addMember(&badMember), "Member moi khong duoc co diem ban dau");

        auth.logout();
        bool permissionDenied = false;
        try { movieService.addMovie(&movieInput); }
        catch (const exception&) { permissionDenied = true; }
        check(permissionDenied, "Khong dang nhap thi khong duoc them Movie");

        cout << "\n===== SUMMARY =====\n";
        cout << "Passed: " << passed << '\n';
        cout << "Failed: " << failed << '\n';
        cout << "Total: " << passed + failed << '\n';
        return failed == 0 ? 0 : 1;
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] " << e.what() << '\n';
        return 2;
    }
}
