
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>

#include "../src/business/MovieService.h"

using namespace std;

int passed = 0;
int failed = 0;

const string USER_FILE = "test_service_users.txt";
const string MOVIE_FILE = "test_service_movies.txt";

void check(const string& name, bool actual, bool expected)
{
    if (actual == expected)
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

void writeTestFiles()
{
    ofstream users(USER_FILE);
    users << "USR0001|admin|AdminPass|Admin User|ADMIN|1\n";
    users << "USR0002|staff|StaffPass|Staff User|STAFF|1\n";
    users.close();

    ofstream movies(MOVIE_FILE);
    movies << "MOV0001|Avatar|Action|120|13|2026-01-01|2026-12-31|1\n";
    movies.close();
}

void testNotLoggedIn()
{
    cout << "\n===== TEST NOT LOGGED IN =====\n";

    UserRepository userRepo(USER_FILE);
    MovieRepository movieRepo(MOVIE_FILE);

    userRepo.load();
    movieRepo.load();

    AuthService authService(userRepo);
    MovieService movieService(movieRepo, authService);

    bool threw = false;

    try
    {
        movieService.findMovieById("MOV0001");
    }
    catch (const runtime_error& e)
    {
        threw = true;
    }

    check("Chua dang nhap khong duoc tim phim", threw, true);

    threw = false;

    try
    {
        movieService.getMovieAt(0);
    }
    catch (const runtime_error& e)
    {
        threw = true;
    }

    check("Chua dang nhap khong duoc lay phim", threw, true);

    threw = false;

    try
    {
        movieService.getMovieCount();
    }
    catch (const runtime_error& e)
    {
        threw = true;
    }

    check("Chua dang nhap khong duoc dem phim", threw, true);
}

void testAdmin()
{
    cout << "\n===== TEST ADMIN =====\n";

    UserRepository userRepo(USER_FILE);
    MovieRepository movieRepo(MOVIE_FILE);

    userRepo.load();
    movieRepo.load();

    AuthService authService(userRepo);
    MovieService movieService(movieRepo, authService);

    // Dang nhap Admin
    check("Admin dang nhap",
          authService.login("admin", "AdminPass"), true);

    // Tim phim theo ID
    const Movie* movie = movieService.findMovieById("MOV0001");

    check("Admin tim phim",
          movie != nullptr, true);

    // Lay phim theo vi tri
    const Movie* movieAt = movieService.getMovieAt(0);

    check("Admin lay phim",
          movieAt != nullptr, true);

    // Lay so luong phim
    check("Admin dem phim",
          movieService.getMovieCount() == 1, true);
}

void testStaff()
{
    cout << "\n===== TEST STAFF =====\n";

    UserRepository userRepo(USER_FILE);
    MovieRepository movieRepo(MOVIE_FILE);

    userRepo.load();
    movieRepo.load();

    AuthService authService(userRepo);
    MovieService movieService(movieRepo, authService);

    // Dang nhap Staff
    check("Staff dang nhap",
          authService.login("staff", "StaffPass"), true);

    // Staff duoc tra cuu
    const Movie* movie = movieService.findMovieById("MOV0001");

    check("Staff tim phim",
          movie != nullptr, true);

    check("Staff lay phim",
          movieService.getMovieAt(0) != nullptr, true);

    check("Staff dem phim",
          movieService.getMovieCount() == 1, true);

    // Staff khong duoc them phim
    Movie* newMovie = new Movie(
        "MOV0002",
        "Titanic",
        "Drama",
        150,
        13,
        "2026-01-01",
        "2026-12-31",
        true
    );

    bool threw = false;

    try
    {
        movieService.addMovie(newMovie);
    }
    catch (const runtime_error& e)
    {
        threw = true;
    }

    check("Staff khong duoc them phim", threw, true);

    // Kiem tra phim khong bi them vao Repository
    check("Staff khong lam thay doi du lieu",
          movieService.getMovieCount() == 1, true);

    // Repository khong nhan quyen so huu con tro khi them that bai
    delete newMovie;
}

#include "../src/models/Showtime.h"

// Kiem tra mot thao tac co nem runtime_error hay khong
template <typename Action>
bool throwsRuntimeError(Action action)
{
    try
    {
        action();
    }
    catch (const runtime_error&)
    {
        return true;
    }
    return false;
}

// Tao phim mau
Movie* createTestMovie(const string& id, int duration = 120)
{
    return new Movie(
        id, "Test Movie", "Action", duration, 13,
        "2026-01-01", "2026-12-31", true
    );
}

// Test Admin them phim
void testAddMovie()
{
    cout << "\n===== TEST ADD MOVIE =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    check("Admin login", auth.login("admin", "AdminPass"), true);

    Movie* movie = createTestMovie("MOV0002");
    bool added = service.addMovie(movie);
    check("Admin them phim", added, true);

    if (!added)
    {
        delete movie;
    }

    check("Tim phim vua them",
          service.findMovieById("MOV0002") != nullptr, true);

    Movie* duplicate = createTestMovie("MOV0002");
    bool duplicateAdded = service.addMovie(duplicate);
    check("Khong them trung ID", duplicateAdded, false);
    if (!duplicateAdded)
    {
        delete duplicate;
    }

    Movie* invalid = createTestMovie("MOV0003", 0);
    bool invalidAdded = service.addMovie(invalid);
    check("Khong them phim sai thoi luong", invalidAdded, false);
    if (!invalidAdded)
    {
        delete invalid;
    }

    check("So luong sau khi them",
          service.getMovieCount() == 2, true);
}

// Test Admin cap nhat phim
void testUpdateMovie()
{
    cout << "\n===== TEST UPDATE MOVIE =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    auth.login("admin", "AdminPass");

    Movie updated(
        "MOV0001", "Avatar Updated", "Sci-Fi",
        150, 16, "2026-01-01", "2026-12-31", true
    );

    check("Admin update phim", service.updateMovie(updated), true);

    const Movie* movie = service.findMovieById("MOV0001");

    check("Ten phim sau update",
          movie != nullptr && movie->getTitle() == "Avatar Updated", true);

    Movie changedActive(
        "MOV0001", "Avatar Updated", "Sci-Fi",
        150, 16, "2026-01-01", "2026-12-31", false
    );

    check("Khong cho doi active",
          service.updateMovie(changedActive), false);

    check("Active khong bi thay doi",
          service.findMovieById("MOV0001")->isActive(), true);

    Movie missing(
        "MOV9999", "Missing", "Action",
        120, 13, "2026-01-01", "2026-12-31", true
    );

    check("Khong update phim khong ton tai",
          service.updateMovie(missing), false);

    Movie invalid(
        "MOV0001", "Invalid", "Action",
        0, 13, "2026-01-01", "2026-12-31", true
    );

    check("Khong update du lieu sai",
          service.updateMovie(invalid), false);

    check("Du lieu cu duoc giu nguyen",
          service.findMovieById("MOV0001")->getTitle() == "Avatar Updated",
          true);
}

// Test rang buoc Showtime
void testShowtimeRules()
{
    cout << "\n===== TEST SHOWTIME RULES =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    auth.login("admin", "AdminPass");

    // Showtime duoc tao tren stack va ton tai
    // trong suot thoi gian Movie su dung no.
    Showtime showtime(
        "SHW0001", "MOV0001", "ROM0001",
        "2026-06-01 10:00", "2026-06-01 12:00",
        ShowtimeStatus::SCHEDULED
    );

    Movie* movie = movies.findById("MOV0001");

    check("Lien ket Showtime",
          showtime.linkMovie(movie), true);

    check("Phim co mot Showtime",
          movie->getShowtimeCount() == 1, true);

    Movie changedDates(
        "MOV0001", "Changed Dates", "Action",
        120, 13, "2026-02-01", "2026-12-31", true
    );

    check("Khong doi ngay khi co Showtime",
          service.updateMovie(changedDates), false);

    check("Ngay phat hanh khong thay doi",
          movie->getReleaseDate() == "2026-01-01", true);

    // Khong doi ngay thi van duoc sua thong tin khac
    Movie changedTitle(
        "MOV0001", "New Title", "Action",
        120, 13, "2026-01-01", "2026-12-31", true
    );

    check("Co Showtime van duoc doi ten",
          service.updateMovie(changedTitle), true);

    check("Khong xoa phim co Showtime",
          service.removeMovie("MOV0001"), false);

    check("Phim van ton tai",
          service.findMovieById("MOV0001") != nullptr, true);
}

// Test Admin xoa phim
void testRemoveMovie()
{
    cout << "\n===== TEST REMOVE MOVIE =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    auth.login("admin", "AdminPass");

    check("Xoa phim khong ton tai",
          service.removeMovie("MOV9999"), false);

    check("Admin xoa phim",
          service.removeMovie("MOV0001"), true);

    check("Phim da bi xoa",
          service.findMovieById("MOV0001") == nullptr, true);

    check("So luong sau khi xoa",
          service.getMovieCount() == 0, true);
}

// Test Staff khong duoc sua, xoa
void testStaffPermissions()
{
    cout << "\n===== TEST STAFF PERMISSIONS =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    auth.login("staff", "StaffPass");

    Movie updated(
        "MOV0001", "Hacked", "Action",
        120, 13, "2026-01-01", "2026-12-31", true
    );

    check("Staff khong duoc update",
          throwsRuntimeError([&]()
          {
              service.updateMovie(updated);
          }), true);

    check("Staff khong duoc xoa",
          throwsRuntimeError([&]()
          {
              service.removeMovie("MOV0001");
          }), true);

    check("Du lieu khong thay doi",
          service.findMovieById("MOV0001")->getTitle() == "Avatar",
          true);

    check("Phim khong bi xoa",
          service.getMovieCount() == 1, true);
}

// Test chua dang nhap khong duoc quan ly phim
void testAnonymousPermissions()
{
    cout << "\n===== TEST ANONYMOUS PERMISSIONS =====\n";

    UserRepository users(USER_FILE);
    MovieRepository movies(MOVIE_FILE);
    users.load();
    movies.load();

    AuthService auth(users);
    MovieService service(movies, auth);

    Movie* movie = createTestMovie("MOV0002");

    check("Anonymous khong duoc them",
          throwsRuntimeError([&]()
          {
              service.addMovie(movie);
          }), true);

    delete movie;

    Movie updated(
        "MOV0001", "Changed", "Action",
        120, 13, "2026-01-01", "2026-12-31", true
    );

    check("Anonymous khong duoc update",
          throwsRuntimeError([&]()
          {
              service.updateMovie(updated);
          }), true);

    check("Anonymous khong duoc xoa",
          throwsRuntimeError([&]()
          {
              service.removeMovie("MOV0001");
          }), true);

    check("Du lieu khong thay doi",
          movies.count() == 1 &&
          movies.findById("MOV0001")->getTitle() == "Avatar",
          true);
}


int main()
{
    writeTestFiles();

    testNotLoggedIn();
    testAdmin();
    testStaff();

    testAddMovie();
    testUpdateMovie();
    testShowtimeRules();
    testRemoveMovie();
    testStaffPermissions();
    testAnonymousPermissions();

    remove(USER_FILE.c_str());
    remove(MOVIE_FILE.c_str());

    cout << "\nPassed: " << passed << endl;
    cout << "Failed: " << failed << endl;

    return failed == 0 ? 0 : 1;
}

