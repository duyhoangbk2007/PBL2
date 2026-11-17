
#include <iostream>
#include <string>
#include "../src/models/Movie.h"
#include "../src/models/Room.h"
#include "../src/models/Showtime.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    if (condition)
    {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name << endl;
        failed++;
    }
}

int main()
{
    Movie movie1(
        "MOV0001", "Movie 1", "Action",
        120, 13, "2026-01-01", "2026-12-31", true
    );

    Movie movie2(
        "MOV0002", "Movie 2", "Comedy",
        90, 0, "2026-01-01", "2026-12-31", true
    );

    Room room1("ROM0001", "Room 1", 5, 10, true);
    Room room2("ROM0002", "Room 2", 5, 10, true);

    Showtime showtime(
        "SHW0001",
        "MOV0001",
        "ROM0001",
        "2026-12-01T18:00:00",
        "2026-12-01T20:00:00",
        ShowtimeStatus::SCHEDULED
    );

    // 1. Trang thai ban dau
    check(showtime.getMovie() == nullptr,
          "Showtime chua lien ket Movie");

    check(showtime.getRoom() == nullptr,
          "Showtime chua lien ket Room");

    // 2. Lien ket
    check(showtime.linkMovie(&movie1),
          "Link Movie thanh cong");

    check(showtime.linkRoom(&room1),
          "Link Room thanh cong");

    check(showtime.getMovie() == &movie1,
          "Showtime tro dung Movie");

    check(showtime.getRoom() == &room1,
          "Showtime tro dung Room");

    check(movie1.getShowtimeCount() == 1,
          "Movie co 1 Showtime");

    check(room1.getShowtimeCount() == 1,
          "Room co 1 Showtime");

    check(movie1.findShowtime("SHW0001") == &showtime,
          "Tim Showtime trong Movie");

    check(room1.findShowtime("SHW0001") == &showtime,
          "Tim Showtime trong Room");

    // 3. Khong cho thao lien ket sai
    check(!movie2.unlinkShowtime(&showtime),
          "Khong unlink tu Movie khac");

    check(!room2.unlinkShowtime(&showtime),
          "Khong unlink tu Room khac");

    check(!movie1.unlinkShowtime(nullptr),
          "Khong unlink Movie voi nullptr");

    check(!room1.unlinkShowtime(nullptr),
          "Khong unlink Room voi nullptr");

    // 4. Thao lien ket Movie
    check(movie1.unlinkShowtime(&showtime),
          "Unlink Movie thanh cong");

    check(showtime.getMovie() == nullptr,
          "Con tro Movie duoc xoa");

    check(movie1.getShowtimeCount() == 0,
          "Danh sach Movie rong");

    check(movie1.findShowtime("SHW0001") == nullptr,
          "Movie khong con Showtime");

    check(showtime.getRoom() == &room1,
          "Unlink Movie khong anh huong Room");

    check(!movie1.unlinkShowtime(&showtime),
          "Khong unlink Movie hai lan");

    // 5. Thao lien ket Room
    check(room1.unlinkShowtime(&showtime),
          "Unlink Room thanh cong");

    check(showtime.getRoom() == nullptr,
          "Con tro Room duoc xoa");

    check(room1.getShowtimeCount() == 0,
          "Danh sach Room rong");

    check(room1.findShowtime("SHW0001") == nullptr,
          "Room khong con Showtime");

    check(!room1.unlinkShowtime(&showtime),
          "Khong unlink Room hai lan");

    // 6. Lien ket lai
    check(movie1.linkShowtime(&showtime),
          "Relink Movie thanh cong");

    check(room1.linkShowtime(&showtime),
          "Relink Room thanh cong");

    check(movie1.getShowtimeCount() == 1,
          "Movie co 1 Showtime sau relink");

    check(room1.getShowtimeCount() == 1,
          "Room co 1 Showtime sau relink");

    // 7. Don dep
    check(movie1.unlinkShowtime(&showtime),
          "Cleanup Movie");

    check(room1.unlinkShowtime(&showtime),
          "Cleanup Room");

    cout << "\n========================\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "========================\n";

    return failed == 0 ? 0 : 1;
}
