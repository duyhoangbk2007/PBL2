
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "../src/business/ShowtimeService.h"
#include "../src/utils/DateTime.h"

using namespace std;

int passed = 0;
int failed = 0;

const string USER_FILE = "test_showtime_users_temp.txt";
const string MOVIE_FILE = "test_showtime_movies_temp.txt";
const string ROOM_FILE = "test_showtime_rooms_temp.txt";
const string SHOWTIME_FILE = "test_showtime_data_temp.txt";
const string PERMISSION_FILE = "test_showtime_permissions_temp.txt";

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

void writeFile(const string& path, const string& content)
{
    ofstream out(path);
    if (!out.is_open())
    {
        throw runtime_error("Khong tao duoc file: " + path);
    }
    out << content;
}

void cleanFiles()
{
    remove(USER_FILE.c_str());
    remove(MOVIE_FILE.c_str());
    remove(ROOM_FILE.c_str());
    remove(SHOWTIME_FILE.c_str());
    remove(PERMISSION_FILE.c_str());
}

void prepareFiles()
{
    cleanFiles();

    writeFile(USER_FILE,
        "USR0001|admin|123456|Quan tri vien|ADMIN|1\n"
        "USR0002|staff|123456|Nhan vien|STAFF|1\n");

    writeFile(MOVIE_FILE,
        "MOV0001|Phim 1|Action|120|13|2000-01-01|2099-12-31|1\n"
        "MOV0002|Phim 2|Drama|90|13|2000-01-01|2099-12-31|1\n"
        "MOV0003|Phim 3|Action|120|13|2000-01-01|2099-12-31|0\n");

    writeFile(ROOM_FILE,
        "ROM0001|Phong 1|8|12|1\n"
        "ROM0002|Phong 2|8|12|1\n"
        "ROM0003|Phong 3|8|12|0\n");

    writeFile(SHOWTIME_FILE, "");
    writeFile(PERMISSION_FILE,
        "ADMIN|SHOWTIME_VIEW\nADMIN|SHOWTIME_ADD\nADMIN|SHOWTIME_UPDATE\n"
        "ADMIN|SHOWTIME_CANCEL\nADMIN|SHOWTIME_DELETE\nSTAFF|SHOWTIME_VIEW\n");
}

Showtime* createShowtime(
    const string& id,
    const string& movieId,
    const string& roomId,
    const string& startAt,
    int duration,
    ShowtimeStatus status = ShowtimeStatus::SCHEDULED)
{
    string endAt;

    try
    {
        endAt = DateTime::addMinutes(startAt, duration);
    }
    catch (const exception& e)
    {
        cerr << "[DEBUG] createShowtime failed"
             << " | ID: " << id
             << " | Start: " << startAt
             << " | Duration: " << duration
             << " | Error: " << e.what() << endl;
        throw;
    }

    return new Showtime(
        id, movieId, roomId, startAt, endAt, status
    );
}

// Repository nhan quyen so huu con tro neu add thanh cong.
bool tryAdd(ShowtimeService& service, Showtime* showtime)
{
    bool result = service.addShowtime(showtime);
    if (!result)
    {
        delete showtime;
    }
    return result;
}

void testShowtimeService()
{
    cout << "\n===== TEST SHOWTIME SERVICE =====\n";

    prepareFiles();

    // Cac moc thoi gian dung chung cho tat ca nhom test.
    const string now = DateTime::getCurrentDateTime();
    const string start1 = DateTime::addMinutes(now, 30 * 24 * 60);
    const string end1 = DateTime::addMinutes(start1, 120);

    const string start2 = DateTime::addMinutes(end1, 15);
    const string startTooClose = DateTime::addMinutes(end1, 14);

    const string updatedStart = DateTime::addMinutes(start2, 240);
    const string updatedEnd = DateTime::addMinutes(updatedStart, 120);

    {
        // ShowtimeRepository phai bi huy truoc Movie/Room Repository.
        UserRepository userRepo(USER_FILE);
        RolePermissionRepository permissionRepo(PERMISSION_FILE);
        MovieRepository movieRepo(MOVIE_FILE);
        RoomRepository roomRepo(ROOM_FILE);
        ShowtimeRepository showtimeRepo(
            SHOWTIME_FILE, movieRepo, roomRepo
        );

        AuthService auth(userRepo, permissionRepo);
        ShowtimeService service(
            showtimeRepo, movieRepo, roomRepo, auth
        );

        // ========================================
        // 1. LOAD / SAVE
        // ========================================

        cout << "\n--- 1. LOAD / SAVE ---\n";

        check("Load users", userRepo.load(), true);
        check("Load permissions", permissionRepo.load(), true);

        check("Save truoc load",
              service.save(), false);

        check("Load showtime khi Movie/Room chua load",
              service.load(), false);

        check("Load movies", movieRepo.load(), true);
        check("Load rooms", roomRepo.load(), true);
        check("Load showtimes", service.load(), true);

        check("Khong load showtimes lan hai",
              service.load(), false);

        check("So suat ban dau bang 0",
              showtimeRepo.count() == 0, true);

        // ========================================
        // 2. CHUA DANG NHAP
        // ========================================

        cout << "\n--- 2. CHUA DANG NHAP ---\n";

        Showtime* anonymous = createShowtime(
            "SHW0001", "MOV0001", "ROM0001",
            start1, 120
        );

        check("Chua login khong duoc them",
              throwsError([&]() {
                  service.addShowtime(anonymous);
              }), true);
        delete anonymous;

        Showtime anonymousUpdate(
            "SHW0001", "MOV0001", "ROM0001",
            start1, end1, ShowtimeStatus::SCHEDULED
        );

        check("Chua login khong duoc sua",
              throwsError([&]() {
                  service.updateShowtime(anonymousUpdate);
              }), true);

        check("Chua login khong duoc huy",
              throwsError([&]() {
                  service.cancelShowtime("SHW0001");
              }), true);

        check("Chua login khong duoc xoa",
              throwsError([&]() {
                  service.removeShowtime("SHW0001");
              }), true);

        check("Chua login khong duoc tim",
              throwsError([&]() {
                  service.findShowtimeById("SHW0001");
              }), true);

        check("Chua login khong duoc getAt",
              throwsError([&]() {
                  service.getShowtimeAt(0);
              }), true);

        check("Chua login khong duoc count",
              throwsError([&]() {
                  service.getShowtimeCount();
              }), true);

        // ========================================
        // 3. STAFF
        // ========================================

        cout << "\n--- 3. STAFF ---\n";

        check("Staff login",
              auth.login("staff", "123456"), true);

        Showtime* staffShowtime = createShowtime(
            "SHW0001", "MOV0001", "ROM0001",
            start1, 120
        );

        check("Staff khong duoc them",
              throwsError([&]() {
                  service.addShowtime(staffShowtime);
              }), true);
        delete staffShowtime;

        Showtime staffUpdate(
            "SHW0001", "MOV0001", "ROM0001",
            start1, end1, ShowtimeStatus::SCHEDULED
        );

        check("Staff khong duoc sua",
              throwsError([&]() {
                  service.updateShowtime(staffUpdate);
              }), true);

        check("Staff khong duoc huy",
              throwsError([&]() {
                  service.cancelShowtime("SHW0001");
              }), true);

        check("Staff khong duoc xoa",
              throwsError([&]() {
                  service.removeShowtime("SHW0001");
              }), true);

        check("Staff duoc dem",
              service.getShowtimeCount() == 0, true);

        check("Staff duoc tim",
              service.findShowtimeById("SHW0001") == nullptr,
              true);

        check("Staff duoc getAt",
              service.getShowtimeAt(0) == nullptr, true);

        // ========================================
        // 4. ADMIN - ADD
        // ========================================

        cout << "\n--- 4. ADMIN ADD ---\n";

        check("Admin login",
              auth.login("admin", "123456"), true);

        Showtime* first = createShowtime(
            "SHW0001", "MOV0001", "ROM0001",
            start1, 120
        );

        check("Admin them suat hop le",
              tryAdd(service, first), true);

        check("Dem sau them",
              service.getShowtimeCount() == 1, true);

        check("Tim theo ID",
              service.findShowtimeById("SHW0001") == first,
              true);

        check("GetAt 0",
              service.getShowtimeAt(0) == first, true);

        check("GetAt ngoai pham vi",
              service.getShowtimeAt(99) == nullptr, true);

        check("GetAt index am",
              service.getShowtimeAt(-1) == nullptr, true);

        check("Tim ID khong ton tai",
              service.findShowtimeById("SHW9999") == nullptr,
              true);

        check("Khong them nullptr",
              service.addShowtime(nullptr), false);

        // Khong xoa first: Repository dang so huu no.
        check("Khong them lai cung con tro",
              service.addShowtime(first), false);

        check("Khong them trung ID",
              tryAdd(service, createShowtime(
                  "SHW0001", "MOV0001", "ROM0002",
                  start1, 120)), false);

        check("Khong them ID sai",
              tryAdd(service, createShowtime(
                  "BAD0001", "MOV0001", "ROM0001",
                  start2, 120)), false);

        check("Khong them Movie khong ton tai",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV9999", "ROM0001",
                  start2, 120)), false);

        check("Khong them Room khong ton tai",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM9999",
                  start2, 120)), false);

        check("Khong them Movie inactive",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0003", "ROM0001",
                  start2, 120)), false);

        check("Khong them Room inactive",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM0003",
                  start2, 120)), false);

        check("Khong them suat CANCELLED",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM0001",
                  start2, 120, ShowtimeStatus::CANCELLED)),
              false);

        Showtime* pastShowtime = new Showtime(
    "SHW0002",
    "MOV0001",
    "ROM0001",
    "2020-01-01 10:00",
    "2020-01-01 12:00",
    ShowtimeStatus::SCHEDULED
);

check("Khong them suat qua khu",
      tryAdd(service, pastShowtime), false);

        Showtime* invalidDate = new Showtime(
            "SHW0002", "MOV0001", "ROM0001",
            "2026-99-99 10:00", "2026-99-99 12:00",
            ShowtimeStatus::SCHEDULED
        );

        check("Khong them datetime sai",
              tryAdd(service, invalidDate), false);

        Showtime* wrongDuration = new Showtime(
            "SHW0002", "MOV0001", "ROM0001",
            start2,
            DateTime::addMinutes(start2, 90),
            ShowtimeStatus::SCHEDULED
        );

        check("Khong them sai thoi luong phim",
              tryAdd(service, wrongDuration), false);

        check("Khong them suat trung lich",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM0001",
                  DateTime::addMinutes(start1, 30), 120)),
              false);

        check("Khong them neu nghi 14 phut",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM0001",
                  startTooClose, 120)), false);

        check("Them duoc neu nghi du 15 phut",
              tryAdd(service, createShowtime(
                  "SHW0002", "MOV0001", "ROM0001",
                  start2, 120)), true);

        check("Cung gio nhung khac phong",
              tryAdd(service, createShowtime(
                  "SHW0003", "MOV0002", "ROM0002",
                  start1, 90)), true);

        check("Dem sau khi them 3 suat",
              service.getShowtimeCount() == 3, true);

        // ========================================
        // 5. ADMIN - UPDATE
        // ========================================

        cout << "\n--- 5. ADMIN UPDATE ---\n";

        Showtime validUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            updatedStart, updatedEnd,
            ShowtimeStatus::SCHEDULED
        );

        check("Admin update hop le",
              service.updateShowtime(validUpdate), true);

        const Showtime* second =
            service.findShowtimeById("SHW0002");

        check("Gio bat dau da cap nhat",
              second != nullptr &&
              second->getStartAt() == updatedStart, true);

        check("Gio ket thuc da cap nhat",
              second != nullptr &&
              second->getEndAt() == updatedEnd, true);

        Showtime missing(
            "SHW9999", "MOV0001", "ROM0001",
            updatedStart, updatedEnd,
            ShowtimeStatus::SCHEDULED
        );

        check("Khong update ID khong ton tai",
              service.updateShowtime(missing), false);

        Showtime changedMovie(
            "SHW0002", "MOV0002", "ROM0001",
            updatedStart,
            DateTime::addMinutes(updatedStart, 90),
            ShowtimeStatus::SCHEDULED
        );

        check("Khong doi Movie khi update",
              service.updateShowtime(changedMovie), false);

        Showtime changedRoom(
            "SHW0002", "MOV0001", "ROM0002",
            updatedStart, updatedEnd,
            ShowtimeStatus::SCHEDULED
        );

        check("Khong doi Room khi update",
              service.updateShowtime(changedRoom), false);

        Showtime cancelledUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            updatedStart, updatedEnd,
            ShowtimeStatus::CANCELLED
        );

        check("Khong doi status qua update",
              service.updateShowtime(cancelledUpdate), false);

        Showtime conflictUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            start1, end1,
            ShowtimeStatus::SCHEDULED
        );

        check("Khong update trung lich",
              service.updateShowtime(conflictUpdate), false);

        Showtime wrongEndUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            updatedStart,
            DateTime::addMinutes(updatedStart, 100),
            ShowtimeStatus::SCHEDULED
        );

        check("Khong update sai thoi luong",
              service.updateShowtime(wrongEndUpdate), false);

        Showtime pastUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            "2020-01-01 10:00", "2020-01-01 12:00",
            ShowtimeStatus::SCHEDULED
        );

        check("Khong update ve qua khu",
              service.updateShowtime(pastUpdate), false);

        Showtime invalidUpdate(
            "SHW0002", "MOV0001", "ROM0001",
            "2026-99-99 10:00", "2026-99-99 12:00",
            ShowtimeStatus::SCHEDULED
        );

        check("Khong update datetime sai",
              service.updateShowtime(invalidUpdate), false);

        second = service.findShowtimeById("SHW0002");

        check("Du lieu cu duoc giu sau update loi",
              second != nullptr &&
              second->getStartAt() == updatedStart &&
              second->getEndAt() == updatedEnd &&
              second->getStatus() == ShowtimeStatus::SCHEDULED,
              true);

        // ========================================
        // 6. LIEN KET MOVIE / ROOM
        // ========================================

        cout << "\n--- 6. LIEN KET MOVIE / ROOM ---\n";

        const Showtime* current =
            service.findShowtimeById("SHW0001");

        Movie* movie = movieRepo.findById("MOV0001");
        Room* room = roomRepo.findById("ROM0001");

        check("Showtime lien ket Movie",
              current != nullptr &&
              current->getMovie() == movie, true);

        check("Showtime lien ket Room",
              current != nullptr &&
              current->getRoom() == room, true);

        check("Movie co 2 suat",
              movie != nullptr &&
              movie->getShowtimeCount() == 2, true);

        check("Room co 2 suat",
              room != nullptr &&
              room->getShowtimeCount() == 2, true);

        check("Movie tim duoc suat",
              movie != nullptr &&
              movie->findShowtime("SHW0001") == current,
              true);

        check("Room tim duoc suat",
              room != nullptr &&
              room->findShowtime("SHW0001") == current,
              true);

        // ========================================
        // 7. CANCEL / REMOVE
        // ========================================

        cout << "\n--- 7. CANCEL / REMOVE ---\n";

        // Hai nghiep vu nay chua hoan thien.
        // Hien tai phai tu choi de tranh sai du lieu ve.

        check("Cancel hien tai tra false",
              service.cancelShowtime("SHW0001"), false);

        check("Remove hien tai tra false",
              service.removeShowtime("SHW0001"), false);

        check("Cancel ID khong ton tai",
              service.cancelShowtime("SHW9999"), false);

        check("Remove ID khong ton tai",
              service.removeShowtime("SHW9999"), false);

        current = service.findShowtimeById("SHW0001");

        check("Status khong doi sau cancel",
              current != nullptr &&
              current->getStatus() == ShowtimeStatus::SCHEDULED,
              true);

        check("Khong mat suat sau remove",
              service.getShowtimeCount() == 3, true);

        check("Lien ket khong doi sau remove",
              movie->getShowtimeCount() == 2 &&
              room->getShowtimeCount() == 2,
              true);

        // ========================================
        // 8. SAVE / LOGOUT
        // ========================================

        cout << "\n--- 8. SAVE / LOGOUT ---\n";

        check("Save showtimes", service.save(), true);

        auth.logout();

        check("Logout mat quyen xem",
              throwsError([&]() {
                  service.getShowtimeCount();
              }), true);

        check("Logout mat quyen sua",
              throwsError([&]() {
                  service.updateShowtime(validUpdate);
              }), true);
    }

    // ========================================
    // 9. RELOAD
    // ========================================

    cout << "\n--- 9. RELOAD ---\n";

    {
        MovieRepository movieRepo(MOVIE_FILE);
        RoomRepository roomRepo(ROOM_FILE);

        check("Reload movies", movieRepo.load(), true);
        check("Reload rooms", roomRepo.load(), true);

        ShowtimeRepository showtimeRepo(
            SHOWTIME_FILE, movieRepo, roomRepo
        );

        check("Reload showtimes",
              showtimeRepo.load(), true);

        check("So suat sau reload",
              showtimeRepo.count() == 3, true);

        Showtime* first =
            showtimeRepo.findById("SHW0001");

        Showtime* second =
            showtimeRepo.findById("SHW0002");

        Showtime* third =
            showtimeRepo.findById("SHW0003");

        check("Suat 1 ton tai",
              first != nullptr, true);

        check("Suat 2 ton tai",
              second != nullptr, true);

        check("Suat 3 ton tai",
              third != nullptr, true);

        check("Gio update duoc luu",
              second != nullptr &&
              second->getStartAt() == updatedStart &&
              second->getEndAt() == updatedEnd,
              true);

        check("Status duoc luu",
              first != nullptr &&
              first->getStatus() == ShowtimeStatus::SCHEDULED,
              true);

        check("Lien ket Movie sau reload",
              first != nullptr &&
              first->getMovie() == movieRepo.findById("MOV0001"),
              true);

        check("Lien ket Room sau reload",
              first != nullptr &&
              first->getRoom() == roomRepo.findById("ROM0001"),
              true);

        Movie* movie = movieRepo.findById("MOV0001");
        Room* room = roomRepo.findById("ROM0001");

        check("Movie co du 2 suat sau reload",
              movie != nullptr &&
              movie->getShowtimeCount() == 2,
              true);

        check("Room co du 2 suat sau reload",
              room != nullptr &&
              room->getShowtimeCount() == 2,
              true);
    }

    cleanFiles();
}

int main()
{
    try
    {
        testShowtimeService();
    }
    catch (const exception& e)
    {
        cout << "[FAIL] Exception: " << e.what() << endl;
        failed++;
        cleanFiles();
    }
    catch (...)
    {
        cout << "[FAIL] Unknown exception" << endl;
        failed++;
        cleanFiles();
    }

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
