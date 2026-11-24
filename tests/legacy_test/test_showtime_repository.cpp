#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include "../src/data_access/ShowtimeRepository.h"
#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/models/Movie.h"
#include "../src/models/Room.h"
using namespace std;

const string MOVIE_FILE = "test_showtime_movies_temp.txt";
const string ROOM_FILE = "test_showtime_rooms_temp.txt";
const string SHOW_FILE = "test_showtimes_temp.txt";
int passed = 0, failed = 0;
void check(const string& name, bool actual, bool expected = true) {
    if (actual == expected) { cout << "[PASS] " << name << '\n'; ++passed; }
    else { cout << "[FAIL] " << name << " | Expected: " << expected << " | Actual: " << actual << '\n'; ++failed; }
}
void writeFile(const string& path, const string& content) { ofstream out(path); out << content; }
void cleanup() { remove(MOVIE_FILE.c_str()); remove(ROOM_FILE.c_str()); remove(SHOW_FILE.c_str()); }
void prepare(const string& showtimes = "") {
    writeFile(MOVIE_FILE, "MOV0001|Phim 1|Action|120|13|2026-01-01|2028-12-31|1\nMOV0002|Phim 2|Comedy|90|0|2026-01-01|2028-12-31|1\n");
    writeFile(ROOM_FILE, "ROM0001|Phong 1|8|12|1\nROM0002|Phong 2|8|12|1\n");
    writeFile(SHOW_FILE, showtimes);
}
Showtime* makeShow(const string& id, const string& movie = "MOV0001", const string& room = "ROM0001",
                   const string& start = "2027-01-10T10:00:00", const string& end = "2027-01-10T12:00:00",
                   ShowtimeStatus status = ShowtimeStatus::SCHEDULED) {
    return new Showtime(id, movie, room, start, end, status);
}
void testBasic() {
    cout << "\n===== BASIC / ADD / UPDATE / REMOVE =====\n";
    prepare();
    MovieRepository movies(MOVIE_FILE);
    RoomRepository rooms(ROOM_FILE);
    ShowtimeRepository shows(SHOW_FILE, movies, rooms);
    check("Chua load", !shows.isLoaded());
    check("Khong load khi Movie/Room chua load", !shows.load());
    check("Khong save truoc load", !shows.save());
    check("Movie load", movies.load());
    check("Room load", rooms.load());
    check("Showtime load file rong", shows.load());
    check("Da load", shows.isLoaded());
    check("Danh sach rong", shows.count() == 0);
    check("Khong load lai", !shows.load());
    check("Khong add nullptr", !shows.add(nullptr));
    Showtime* a = makeShow("SHW0001");
    check("Them suat hop le", shows.add(a));
    check("So luong sau add", shows.count() == 1);
    check("Find theo ID", shows.findById("SHW0001") == a);
    check("GetAt 0", shows.getAt(0) == a);
    check("GetAt ngoai pham vi", shows.getAt(100) == nullptr);
    check("Find khong ton tai", shows.findById("SHW9999") == nullptr);
    check("Lien ket Movie", a->getMovie() == movies.findById("MOV0001"));
    check("Lien ket Room", a->getRoom() == rooms.findById("ROM0001"));
    check("Movie tim duoc suat", movies.findById("MOV0001")->findShowtime("SHW0001") == a);
    check("Room tim duoc suat", rooms.findById("ROM0001")->findShowtime("SHW0001") == a);
    check("Khong add lai con tro", !shows.add(a));
    Showtime* duplicate = makeShow("SHW0001");
    bool ok = shows.add(duplicate);
    check("Khong trung ID", !ok); if (!ok) delete duplicate;
    Showtime* bad = makeShow("SHW0000"); ok = shows.add(bad);
    check("Khong ID sai", !ok); if (!ok) delete bad;
    bad = makeShow("SHW0002", "MOV9999"); ok = shows.add(bad);
    check("Khong Movie khong ton tai", !ok); if (!ok) delete bad;
    bad = makeShow("SHW0002", "MOV0001", "ROM9999"); ok = shows.add(bad);
    check("Khong Room khong ton tai", !ok); if (!ok) delete bad;
    bad = makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10T12:00:00", "2027-01-10T10:00:00");
    ok = shows.add(bad); check("Khong end truoc start", !ok); if (!ok) delete bad;
    bad = makeShow("SHW0002", "MOV0001", "ROM0001", "2027-01-10 12:00", "2027-01-10T14:00:00");
    ok = shows.add(bad); check("Khong sai format datetime", !ok); if (!ok) delete bad;
    Showtime* overlap = makeShow("SHW0002", "MOV0002", "ROM0001", "2027-01-10T12:14:00", "2027-01-10T13:44:00");
    ok = shows.add(overlap); check("Khong cho nghi 14 phut", !ok); if (!ok) delete overlap;
    Showtime* exact = makeShow("SHW0002", "MOV0002", "ROM0001", "2027-01-10T12:15:00", "2027-01-10T13:45:00");
    check("Cho nghi dung 15 phut", shows.add(exact));
    Showtime* otherRoom = makeShow("SHW0003", "MOV0002", "ROM0002", "2027-01-10T10:00:00", "2027-01-10T11:30:00");
    check("Cung gio khac phong", shows.add(otherRoom));
    Showtime* cancelled = makeShow("SHW0004", "MOV0001", "ROM0001", "2027-01-10T10:00:00", "2027-01-10T12:00:00", ShowtimeStatus::CANCELLED);
    check("Suat CANCELLED khong trung lich", shows.add(cancelled));
    Showtime updateOk("SHW0002", "MOV0002", "ROM0001", "2027-01-10T13:00:00", "2027-01-10T14:30:00", ShowtimeStatus::SCHEDULED);
    check("Update thoi gian", shows.update(updateOk));
    check("Thoi gian sau update", exact->getStartAt() == "2027-01-10T13:00:00");
    Showtime updateConflict("SHW0002", "MOV0002", "ROM0001", "2027-01-10T11:00:00", "2027-01-10T12:30:00", ShowtimeStatus::SCHEDULED);
    check("Update trung lich bi chan", !shows.update(updateConflict));
    check("Update loi khong doi du lieu", exact->getStartAt() == "2027-01-10T13:00:00");
    Showtime updateRoom("SHW0002", "MOV0002", "ROM0002", "2027-01-10T13:00:00", "2027-01-10T14:30:00", ShowtimeStatus::SCHEDULED);
    check("Khong cho doi Room", !shows.update(updateRoom));
    Showtime updateMovie("SHW0002", "MOV0001", "ROM0001", "2027-01-10T13:00:00", "2027-01-10T14:30:00", ShowtimeStatus::SCHEDULED);
    check("Khong cho doi Movie", !shows.update(updateMovie));
    Showtime missing("SHW9999", "MOV0001", "ROM0001", "2027-01-10T13:00:00", "2027-01-10T15:00:00", ShowtimeStatus::SCHEDULED);
    check("Khong update ID khong ton tai", !shows.update(missing));
    check("Khong xoa Movie con suat", !movies.removeById("MOV0001"));
    check("Khong xoa Room con suat", !rooms.removeById("ROM0001"));
    check("Save", shows.save());
    check("Remove suat", shows.removeById("SHW0001"));
    check("Movie da unlink", movies.findById("MOV0001")->findShowtime("SHW0001") == nullptr);
    check("Room da unlink", rooms.findById("ROM0001")->findShowtime("SHW0001") == nullptr);
    check("Khong remove lai", !shows.removeById("SHW0001"));
    check("Count sau remove", shows.count() == 3);
    // shows destructor runs before rooms/movies
}
void testLoadAndInvalidFiles() {
    cout << "\n===== LOAD / FILE INVALID / ROLLBACK =====\n";
    prepare("SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n"
            "SHW0002|MOV0002|ROM0001|2027-01-10T12:15:00|2027-01-10T13:45:00|SCHEDULED\n");
    {
        MovieRepository movies(MOVIE_FILE); RoomRepository rooms(ROOM_FILE);
        check("Load Movies", movies.load()); check("Load Rooms", rooms.load());
        ShowtimeRepository shows(SHOW_FILE, movies, rooms);
        check("Load 2 suat", shows.load());
        check("Count = 2", shows.count() == 2);
        check("Movie co suat", movies.findById("MOV0001")->getShowtimeCount() == 1);
        check("Room co 2 suat", rooms.findById("ROM0001")->getShowtimeCount() == 2);
        check("Load lai bi chan", !shows.load());
    }
    const string cases[][2] = {
        {"Thieu truong", "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00\n"},
        {"Trang thai sai", "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|INVALID\n"},
        {"Ngay gio sai", "SHW0001|MOV0001|ROM0001|2027-01-10 10:00|2027-01-10T12:00:00|SCHEDULED\n"},
        {"Trung ID", "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\nSHW0001|MOV0001|ROM0002|2027-01-10T13:00:00|2027-01-10T15:00:00|SCHEDULED\n"},
        {"Movie khong ton tai", "SHW0001|MOV9999|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n"},
        {"Room khong ton tai", "SHW0001|MOV0001|ROM9999|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n"},
        {"Trung lich", "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\nSHW0002|MOV0002|ROM0001|2027-01-10T12:14:00|2027-01-10T13:44:00|SCHEDULED\n"}
    };
    for (const auto& test : cases) {
        prepare(test[1]);
        MovieRepository movies(MOVIE_FILE); RoomRepository rooms(ROOM_FILE);
        movies.load(); rooms.load();
        ShowtimeRepository shows(SHOW_FILE, movies, rooms);
        bool threw = false;
        try { shows.load(); } catch (const exception&) { threw = true; }
        check(string("Load loi: ") + test[0], threw);
        check(string("Rollback: ") + test[0], !shows.isLoaded() && shows.count() == 0);
        check(string("Movie khong con link: ") + test[0], movies.findById("MOV0001")->getShowtimeCount() == 0);
        check(string("Room khong con link: ") + test[0], rooms.findById("ROM0001")->getShowtimeCount() == 0);
    }
    prepare();
    remove(SHOW_FILE.c_str());
    MovieRepository movies(MOVIE_FILE); RoomRepository rooms(ROOM_FILE);
    movies.load(); rooms.load();
    ShowtimeRepository shows(SHOW_FILE, movies, rooms);
    check("Load file khong ton tai", !shows.load());
    check("Khong loaded khi file khong ton tai", !shows.isLoaded());
}
void testInactiveAndLinked() {
    cout << "\n===== INACTIVE / PRELINKED =====\n";
    prepare();
    MovieRepository movies(MOVIE_FILE); RoomRepository rooms(ROOM_FILE);
    movies.load(); rooms.load();
    ShowtimeRepository shows(SHOW_FILE, movies, rooms); shows.load();
    Movie* movie = movies.findById("MOV0001");
    Room* room = rooms.findById("ROM0001");
    movie->setActive(false);
    Showtime* a = makeShow("SHW0001"); bool ok = shows.add(a);
    check("SCHEDULED bi chan neu Movie inactive", !ok); if (!ok) delete a;
    movie->setActive(true); room->setActive(false);
    a = makeShow("SHW0001"); ok = shows.add(a);
    check("SCHEDULED bi chan neu Room inactive", !ok); if (!ok) delete a;
    room->setActive(true);
    a = makeShow("SHW0001");
    check("Link Movie truoc", a->linkMovie(movie));
    check("Khong add suat da link truoc", !shows.add(a));
    check("Lien ket cu khong bi pha", a->getMovie() == movie);
    check("Unlink thu cong", movie->unlinkShowtime(a));
    delete a;
}
int main() {
    testBasic(); testLoadAndInvalidFiles(); testInactiveAndLinked();
    cleanup();
    cout << "\n===== SUMMARY =====\nPassed: " << passed << "\nFailed: " << failed << "\nTotal: " << passed + failed << '\n';
    return failed == 0 ? 0 : 1;
}
