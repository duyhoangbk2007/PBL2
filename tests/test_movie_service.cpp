#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <ctime>
#include "../src/business/MovieService.h"
#include "../src/models/Showtime.h"
#include "../src/utils/DateTime.h"

using namespace std;

int passed = 0;
int failed = 0;
const string USER_FILE = "test_service_users.txt";
const string MOVIE_FILE = "test_service_movies.txt";
const string PERMISSION_FILE = "test_service_permissions.txt";

void check(const string& name, bool actual, bool expected = true)
{
    if (actual == expected)
    {
        cout << "[PASS] " << name << '\n';
        ++passed;
    }
    else
    {
        cout << "[FAIL] " << name << " (actual=" << actual
             << ", expected=" << expected << ")\n";
        ++failed;
    }
}

template <typename Action>
bool throwsRuntimeError(Action action)
{
    try { action(); }
    catch (const runtime_error&) { return true; }
    return false;
}

void writeFiles(const string& permissions =
    "ADMIN|MOVIE_ADD\nADMIN|MOVIE_UPDATE\nADMIN|MOVIE_DELETE\n"
    "ADMIN|MOVIE_VIEW\nSTAFF|MOVIE_VIEW\n")
{
    ofstream users(USER_FILE, ios::trunc);
    users << "USR0001|admin|AdminPass|Admin User|ADMIN|1\n";
    users << "USR0002|staff|StaffPass|Staff User|STAFF|1\n";
    users.close();

    ofstream movies(MOVIE_FILE, ios::trunc);
    movies << "MOV0001|Avatar|Action|120|13|2020-01-01|2099-12-31|1\n";
    movies.close();

    ofstream perms(PERMISSION_FILE, ios::trunc);
    perms << permissions;
}

Movie* makeMovie(const string& id, const string& title = "Test Movie",
                 int duration = 120, bool active = true,
                 const string& release = "2020-01-01",
                 const string& end = "2099-12-31")
{
    return new Movie(id, title, "Action", duration, 13, release, end, active);
}

// Each fixture gets fresh files and repositories. Destruction order:
// service -> auth -> permissions -> movies -> users.
struct Fixture
{
    UserRepository users;
    MovieRepository movies;
    RolePermissionRepository permissions;
    AuthService auth;
    MovieService service;

    Fixture()
        : users(USER_FILE), movies(MOVIE_FILE), permissions(PERMISSION_FILE),
          auth(users, permissions), service(movies, auth)
    {
        check("Load users", users.load());
        check("Load movies", service.load());
        check("Load permissions", permissions.load());
    }

    void loginAdmin() { check("Admin login", auth.login("admin", "AdminPass")); }
    void loginStaff() { check("Staff login", auth.login("staff", "StaffPass")); }
};

void testBasicCrud()
{
    cout << "\n===== CRUD =====\n";
    writeFiles();
    Fixture f;
    f.loginAdmin();
    check("Initial count", f.service.getMovieCount() == 1);
    check("Find existing", f.service.findMovieById("MOV0001") != nullptr);
    check("Find missing", f.service.findMovieById("MOV9999") == nullptr);
    check("Get at 0", f.service.getMovieAt(0) != nullptr);
    check("Get negative index", f.service.getMovieAt(-1) == nullptr);
    check("Get out of range", f.service.getMovieAt(999) == nullptr);

    Movie* added = makeMovie("MOV0002");
    bool ok = f.service.addMovie(added);
    check("Add valid", ok);
    if (!ok) delete added;
    check("Count after add", f.service.getMovieCount() == 2);

    Movie* duplicate = makeMovie("MOV0002");
    ok = f.service.addMovie(duplicate);
    check("Reject duplicate", !ok);
    if (!ok) delete duplicate;

    Movie* invalid = makeMovie("MOV0003", "Bad", 0);
    ok = f.service.addMovie(invalid);
    check("Reject invalid duration", !ok);
    if (!ok) delete invalid;

    Movie updated("MOV0001", "Avatar Updated", "Sci-Fi", 150, 16,
                  "2021-01-01", "2099-12-31", true);
    check("Update without showtime", f.service.updateMovie(updated));
    const Movie* m = f.service.findMovieById("MOV0001");
    check("Updated fields", m && m->getTitle() == "Avatar Updated" &&
          m->getDurationMinutes() == 150 && m->getReleaseDate() == "2021-01-01");

    Movie wrongActive("MOV0001", "Avatar Updated", "Sci-Fi", 150, 16,
                      "2021-01-01", "2099-12-31", false);
    check("Update cannot change active", !f.service.updateMovie(wrongActive));
    Movie bad("MOV0001", "Bad", "Action", 0, 13,
              "2021-01-01", "2099-12-31", true);
    check("Reject invalid update", !f.service.updateMovie(bad));
    check("Invalid update preserves title",
          f.service.findMovieById("MOV0001")->getTitle() == "Avatar Updated");
    Movie missing("MOV9999", "Missing", "Action", 120, 13,
                  "2020-01-01", "2099-12-31", true);
    check("Reject update missing movie", !f.service.updateMovie(missing));
    check("Reject remove missing", !f.service.removeMovie("MOV9999"));
    check("Remove unlinked movie", f.service.removeMovie("MOV0002"));
    check("Removed movie absent", f.service.findMovieById("MOV0002") == nullptr);
    check("Save", f.service.save());
}

void testShowtimeRules()
{
    cout << "\n===== SHOWTIME / ACTIVE =====\n";
    writeFiles();
    Fixture f;
    f.loginAdmin();
    Movie* movie = f.movies.findById("MOV0001");
    const string now = DateTime::getCurrentDateTime();
    const string future = DateTime::addMinutes(now, 180);

    time_t pastTime = time(nullptr) - 180 * 60;
    tm* pastInfo = localtime(&pastTime);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", pastInfo);

    const string past = buffer;


    Showtime upcoming("SHW0001", "MOV0001", "ROM0001", future,
                      DateTime::addMinutes(future, 120), ShowtimeStatus::SCHEDULED);
    check("Link upcoming showtime", upcoming.linkMovie(movie));
    check("Get showtime at 0", movie->getShowtimeAt(0) == &upcoming);
    check("Get showtime invalid", movie->getShowtimeAt(-1) == nullptr &&
          movie->getShowtimeAt(1) == nullptr);

    Movie changedDuration("MOV0001", "Avatar", "Action", 150, 13,
                          "2020-01-01", "2099-12-31", true);
    check("Block duration change with showtime", !f.service.updateMovie(changedDuration));
    Movie changedDate("MOV0001", "Avatar", "Action", 120, 13,
                      "2021-01-01", "2099-12-31", true);
    check("Block date change with showtime", !f.service.updateMovie(changedDate));
    Movie changedTitle("MOV0001", "New Title", "Action", 120, 13,
                       "2020-01-01", "2099-12-31", true);
    check("Allow title change with showtime", f.service.updateMovie(changedTitle));
    check("Block remove linked movie", !f.service.removeMovie("MOV0001"));
    check("Block deactivate with future scheduled", !f.service.setMovieActive("MOV0001", false));
    check("Still active after rejection", movie->isActive());
    check("Same active state succeeds", f.service.setMovieActive("MOV0001", true));

    upcoming.setStatus(ShowtimeStatus::CANCELLED);
    check("Allow deactivate when future cancelled", f.service.setMovieActive("MOV0001", false));
    check("Inactive persisted in memory", !movie->isActive());
    check("Same inactive state succeeds", f.service.setMovieActive("MOV0001", false));
    check("Reactivate", f.service.setMovieActive("MOV0001", true));
    check("Reactivate changes state", movie->isActive());
    check("Unlink upcoming", movie->unlinkShowtime(&upcoming));

    Showtime historical("SHW0002", "MOV0001", "ROM0001", past,
                        DateTime::addMinutes(past, 120), ShowtimeStatus::SCHEDULED);
    check("Link past showtime", historical.linkMovie(movie));
    check("Allow deactivate with past showtime", f.service.setMovieActive("MOV0001", false));
    check("Cannot remove even with past showtime", !f.service.removeMovie("MOV0001"));
    check("Unlink past showtime", movie->unlinkShowtime(&historical));
    check("Reactivate after unlink", f.service.setMovieActive("MOV0001", true));
    check("Reject setActive missing movie", !f.service.setMovieActive("MOV9999", false));
    check("Remove after unlink", f.service.removeMovie("MOV0001"));
}

void testFilteringAndSearch()
{
    cout << "\n===== FILTER / SEARCH =====\n";
    writeFiles();
    Fixture f;
    f.loginStaff();
    const string today = DateTime::getCurrentDateTime().substr(0, 10);
    // The baseline MOV0001 is active and spans 2020-2099.
    Movie* coming = makeMovie("MOV0002", "Future Batman", 120, true,
                             "2099-12-30", "2099-12-31");
    Movie* inactive = makeMovie("MOV0003", "Hidden Avatar", 120, false);
    Movie* expired = makeMovie("MOV0004", "Old Movie", 120, true,
                              "2000-01-01", "2001-01-01");
    for (Movie* m : {coming, inactive, expired})
    {
        bool ok = f.movies.add(m);
        check("Fixture movie added", ok);
        if (!ok) delete m;
    }
    // Check exact boundary on today using another movie.
    Movie* boundary = makeMovie("MOV0005", "Today Premiere", 120, true,
                                today, today);
    bool ok = f.movies.add(boundary);
    check("Add boundary movie", ok);
    if (!ok) delete boundary;

    vector<const Movie*> now = f.service.getNowShowingMovies();
    vector<const Movie*> soon = f.service.getComingSoonMovies();
    check("Now showing includes baseline and boundary", now.size() == 2);
    check("Coming soon includes future movie", soon.size() == 1 &&
          soon[0]->getMovieId() == "MOV0002");
    check("Case-insensitive search", f.service.searchMoviesByTitle("aVaTaR").size() == 2);
    check("Partial title search", f.service.searchMoviesByTitle("BAT").size() == 1);
    check("Search missing keyword", f.service.searchMoviesByTitle("zzzz").empty());
    check("Empty keyword returns all", f.service.searchMoviesByTitle("").size() == 5);
    check("Search includes inactive movie", f.service.searchMoviesByTitle("Hidden").size() == 1);
}

void testPermissions()
{
    cout << "\n===== PERMISSIONS =====\n";
    writeFiles();
    Fixture f;
    Movie* temp = makeMovie("MOV0002");
    check("Anonymous cannot add", throwsRuntimeError([&]() { f.service.addMovie(temp); }));
    delete temp;
    check("Anonymous cannot view", throwsRuntimeError([&]() { f.service.getMovieCount(); }));
    check("Anonymous cannot search", throwsRuntimeError([&]() { f.service.searchMoviesByTitle("Avatar"); }));
    check("Anonymous cannot filter", throwsRuntimeError([&]() { f.service.getNowShowingMovies(); }));
    check("Anonymous cannot deactivate", throwsRuntimeError([&]() { f.service.setMovieActive("MOV0001", false); }));
    f.loginStaff();
    check("Staff can view", f.service.findMovieById("MOV0001") != nullptr);
    check("Staff can search", f.service.searchMoviesByTitle("Avatar").size() == 1);
    check("Staff cannot deactivate", throwsRuntimeError([&]() { f.service.setMovieActive("MOV0001", false); }));
    Movie updated("MOV0001", "Changed", "Action", 120, 13,
                  "2020-01-01", "2099-12-31", true);
    check("Staff cannot update", throwsRuntimeError([&]() { f.service.updateMovie(updated); }));
    check("Staff cannot remove", throwsRuntimeError([&]() { f.service.removeMovie("MOV0001"); }));
    temp = makeMovie("MOV0002");
    check("Staff cannot add", throwsRuntimeError([&]() { f.service.addMovie(temp); }));
    delete temp;
    check("Unauthorized actions preserve movie", f.movies.findById("MOV0001") != nullptr &&
          f.movies.findById("MOV0001")->isActive());
}

void testCustomPermissions()
{
    cout << "\n===== CUSTOM RBAC =====\n";
    writeFiles("STAFF|MOVIE_ADD\nADMIN|MOVIE_VIEW\n");
    Fixture f;
    f.loginStaff();
    Movie* m = makeMovie("MOV0002");
    bool ok = f.service.addMovie(m);
    check("Staff can add if granted", ok);
    if (!ok) delete m;
    check("Staff cannot view without permission", throwsRuntimeError([&]() { f.service.getMovieCount(); }));
    f.loginAdmin();
    check("Admin can view if granted", f.service.getMovieCount() == 2);
    check("Admin cannot update without permission", throwsRuntimeError([&]() { f.service.setMovieActive("MOV0001", false); }));
    m = makeMovie("MOV0003");
    check("Admin cannot add without permission", throwsRuntimeError([&]() { f.service.addMovie(m); }));
    delete m;
}

int main()
{
    testBasicCrud();
    testShowtimeRules();
    testFilteringAndSearch();
    testPermissions();
    testCustomPermissions();
    remove(USER_FILE.c_str());
    remove(MOVIE_FILE.c_str());
    remove(PERMISSION_FILE.c_str());
    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "Total: " << passed + failed << '\n';
    return failed == 0 ? 0 : 1;
}
