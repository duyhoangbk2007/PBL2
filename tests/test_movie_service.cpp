#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "../src/business/MovieService.h"
#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/models/Showtime.h"
#include "../src/utils/DateTime.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << endl;
    (condition ? passed : failed)++;
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

void writeFile(const fs::path& path, const string& data)
{
    ofstream out(path, ios::binary);
    if (!out || !(out << data))
    {
        throw runtime_error("Khong ghi duoc file test: " + path.string());
    }
}

string readFile(const fs::path& path)
{
    ifstream in(path, ios::binary);
    if (!in)
    {
        throw runtime_error("Khong doc duoc file test: " + path.string());
    }
    string content(istreambuf_iterator<char>{in}, istreambuf_iterator<char>{});
    string normalized;
    for (char ch : content)
    {
        if (ch != '\r')
        {
            normalized += ch;
        }
    }
    return normalized;
}

struct TestFiles
{
    fs::path dir;

    TestFiles()
    {
        static int sequence = 0;
        dir = fs::temp_directory_path() /
              ("pbl2_movie_service_" +
               to_string(chrono::steady_clock::now().time_since_epoch().count()) +
               "_" + to_string(++sequence));
        fs::create_directories(dir);
    }

    ~TestFiles()
    {
        error_code error;
        fs::remove_all(dir, error);
    }

    fs::path path(const string& name) const
    {
        return dir / name;
    }
};

const string USERS =
    "USR0001|admin|secret|Admin User|ADMIN|1\n"
    "USR0002|staff|secret|Staff User|STAFF|1\n";
const string ALL_PERMISSIONS =
    "ADMIN|MOVIE_VIEW\nADMIN|MOVIE_ADD\nADMIN|MOVIE_UPDATE\nADMIN|MOVIE_DELETE\n"
    "STAFF|MOVIE_VIEW\nSTAFF|MOVIE_ADD\nSTAFF|MOVIE_UPDATE\nSTAFF|MOVIE_DELETE\n";

struct Fixture
{
    TestFiles files;
    MovieRepository concrete;
    IRepository<Movie>& repo;
    UserRepository users;
    RolePermissionRepository permissions;
    AuthService auth;
    MovieService service;

    Fixture(const string& movies = "", const string& rights = ALL_PERMISSIONS)
        : concrete(files.path("movies.txt").string()), repo(concrete),
          users(files.path("users.txt").string()),
          permissions(files.path("permissions.txt").string()),
          auth(users, permissions), service(repo, auth)
    {
        writeFile(files.path("movies.txt"), movies);
        writeFile(files.path("users.txt"), USERS);
        writeFile(files.path("permissions.txt"), rights);
        if (!users.load() || !permissions.load())
        {
            throw runtime_error("Khong nap duoc dependency test");
        }
    }

    void login(const string& name = "admin")
    {
        if (!auth.login(name, "secret"))
        {
            throw runtime_error("Khong dang nhap duoc tai khoan test");
        }
    }
};

Movie makeMovie(const string& id, const string& title = "Phim moi",
                const string& start = "2026-01-01",
                const string& end = "2099-12-31", bool active = true,
                int duration = 120)
{
    return Movie(id, title, "Action", duration, 13, start, end, active);
}

string row(const string& id, const string& title, const string& start,
           const string& end, bool active = true)
{
    return id + "|" + title + "|Action|120|13|" + start + "|" + end +
           "|" + (active ? "1" : "0") + "\n";
}

void testInterfaceLoadSaveAndQueries()
{
    cout << "\n===== TEST INTERFACE / LOAD / QUERY =====" << endl;
    check(is_base_of<IRepository<Movie>, MovieRepository>::value &&
          is_constructible<MovieService, IRepository<Movie>&, AuthService&>::value,
          "MovieService nhan IRepository<Movie>& qua constructor");
    Fixture f;
    f.login();
    check(!f.repo.isLoaded() && f.service.getMovieCount() == 0 &&
          f.service.findMovieById("MOV0001") == nullptr &&
          f.service.getMovieAt(0) == nullptr, "Truy van truoc load");
    check(!f.service.save(), "Save truoc load tra false");
    check(f.service.load() && f.repo.isLoaded() && !f.service.load(),
          "Load file rong va load lap");
    check(f.service.save() && readFile(f.files.path("movies.txt")).empty(),
          "Save file rong");
    Movie input = makeMovie("MOV9999");
    check(f.service.addMovie(&input), "Add sau load");
    const Movie* found = f.service.findMovieById("MOV0001");
    check(found && found == f.service.getMovieAt(0) &&
          f.service.getMovieCount() == 1 &&
          f.service.getMovieAt(-1) == nullptr &&
          f.service.getMovieAt(1) == nullptr &&
          f.service.findMovieById("MOV9999") == nullptr,
          "Find, getAt, count va bien index");
    check(f.service.save() && readFile(f.files.path("movies.txt")) ==
          row("MOV0001", "Phim moi", "2026-01-01", "2099-12-31"),
          "Save ban ghi dung schema");

    Fixture missing;
    fs::remove(missing.files.path("movies.txt"));
    check(!missing.service.load() && !missing.repo.isLoaded(), "Load file thieu");
    Fixture invalid("MOV0001|bad\n");
    check(throwsError([&] { invalid.service.load(); }) && !invalid.repo.isLoaded(),
          "Load du lieu sai truyen ngoai le");
}

void testPermissionOnly()
{
    cout << "\n===== TEST PERMISSION-ONLY =====" << endl;
    Fixture f("", "STAFF|MOVIE_VIEW\nSTAFF|MOVIE_ADD\nSTAFF|MOVIE_UPDATE\nSTAFF|MOVIE_DELETE\n");
    check(f.service.load(), "Load khong doi hoi Permission");
    Movie candidate = makeMovie("IGNORED");
    check(throwsError([&] { f.service.addMovie(&candidate); }) &&
          throwsError([&] { f.service.getMovieCount(); }),
          "Chua dang nhap bi tu choi");
    f.login("admin");
    check(throwsError([&] { f.service.addMovie(&candidate); }) &&
          throwsError([&] { f.service.getMovieCount(); }) &&
          f.repo.count() == 0, "ADMIN khong duoc cap quyen bi tu choi");
    f.login("staff");
    check(f.service.addMovie(&candidate) && f.service.getMovieCount() == 1 &&
          f.service.findMovieById("MOV0001") != nullptr,
          "STAFF duoc cap quyen duoc thao tac");
    User* staff = f.users.findById("USR0002");
    staff->setActive(false);
    check(throwsError([&] { f.service.getMovieCount(); }),
          "Tai khoan bi vo hieu hoa bi tu choi");

    Fixture denied("", "STAFF|MOVIE_VIEW\n");
    denied.service.load();
    denied.login("staff");
    check(throwsError([&] { denied.service.addMovie(&candidate); }) &&
          denied.repo.count() == 0, "STAFF thieu MOVIE_ADD bi tu choi");
    check(denied.service.getMovieCount() == 0, "Permission VIEW doc lap voi ADD");

    Fixture noRights(row("MOV0001", "Phim", "2026-01-01", "2099-12-31"), "");
    noRights.service.load();
    noRights.login("admin");
    Movie values = makeMovie("MOV0001");
    check(throwsError([&] { noRights.service.updateMovie(values); }) &&
          throwsError([&] { noRights.service.setMovieActive("MOV0001", false); }) &&
          throwsError([&] { noRights.service.removeMovie("MOV0001"); }) &&
          noRights.repo.count() == 1,
          "ADMIN thieu UPDATE va DELETE: khong doi du lieu");
    check(throwsError([&] { noRights.service.findMovieById("MOV0001"); }) &&
          throwsError([&] { noRights.service.getMovieAt(0); }) &&
          throwsError([&] { noRights.service.getNowShowingMovies(); }) &&
          throwsError([&] { noRights.service.getComingSoonMovies(); }) &&
          throwsError([&] { noRights.service.searchMoviesByTitle("Phim"); }),
          "ADMIN thieu VIEW: moi API tra cuu bi tu choi");
}

void testAddUpdateRemove()
{
    cout << "\n===== TEST ADD / UPDATE / REMOVE =====" << endl;
    Fixture f;
    f.login();
    Movie input = makeMovie("ORIGINAL");
    check(!f.service.addMovie(nullptr) && !f.service.addMovie(&input),
          "Add nullptr va truoc load");
    f.service.load();
    check(f.service.addMovie(&input) && input.getMovieId() == "ORIGINAL" &&
          f.repo.findById("MOV0001") != &input,
          "Sinh MOV0001, sao chep input va caller giu input");
    Movie second = makeMovie("ANY", "Phim hai");
    check(f.service.addMovie(&second) && f.repo.findById("MOV0002") != nullptr,
          "Sinh ID ke tiep");
    Movie bad = makeMovie("ANY", "");
    int count = f.repo.count();
    check(!f.service.addMovie(&bad) && f.repo.count() == count &&
          bad.getMovieId() == "ANY", "Validation fail giu count va input");
    Movie* first = f.repo.findById("MOV0001");
    Movie changed = makeMovie("MOV0001", "Ten moi");
    check(f.service.updateMovie(changed) && f.repo.findById("MOV0001") == first &&
          first->getTitle() == "Ten moi", "Update hop le giu dia chi va ID");
    Movie missing = makeMovie("MOV9999");
    Movie inactive = makeMovie("MOV0001", "Ten khac", "2026-01-01", "2099-12-31", false);
    Movie invalid = makeMovie("MOV0001", "", "2026-01-01", "2099-12-31");
    check(!f.service.updateMovie(missing) && !f.service.updateMovie(inactive) &&
          !f.service.updateMovie(invalid) && first->getTitle() == "Ten moi" &&
          first->isActive() && f.repo.count() == count,
          "Update that bai khong doi du lieu");
    check(!f.service.removeMovie("MOV9999") && f.repo.count() == count,
          "Remove ID thieu giu nguyen du lieu");
    check(f.service.removeMovie("MOV0002") && f.repo.findById("MOV0002") == nullptr &&
          f.repo.count() == 1, "Remove phim khong lien ket");

    Fixture exhausted(row("MOV9999", "Cuoi day", "2026-01-01", "2099-12-31"));
    exhausted.login();
    exhausted.service.load();
    Movie next = makeMovie("ORIGINAL");
    check(throwsError([&] { exhausted.service.addMovie(&next); }) &&
          exhausted.repo.count() == 1 && next.getMovieId() == "ORIGINAL",
          "Het day ID: truyen ngoai le, caller giu input, du lieu khong doi");
}

void testShowtimeRules()
{
    cout << "\n===== TEST SHOWTIME RULES =====" << endl;
    Fixture f(row("MOV0001", "Phim", "2026-01-01", "2099-12-31"));
    f.login();
    f.service.load();
    Movie* movie = f.repo.findById("MOV0001");
    Showtime future("SHW0001", "MOV0001", "ROM0001", "2099-01-01T10:00:00",
                    "2099-01-01T12:00:00", ShowtimeStatus::SCHEDULED);
    check(movie && movie->linkShowtime(&future), "Lien ket Showtime voi Movie");
    Movie duration = makeMovie("MOV0001", "Phim", "2026-01-01", "2099-12-31", true, 90);
    Movie dates = makeMovie("MOV0001", "Phim", "2027-01-01", "2099-12-31");
    check(!f.service.updateMovie(duration) && !f.service.updateMovie(dates) &&
          movie->getDurationMinutes() == 120 && movie->getReleaseDate() == "2026-01-01",
          "Co suat: chan doi thoi luong va ngay");
    check(!f.service.removeMovie("MOV0001") &&
          !f.service.setMovieActive("MOV0001", false) && movie->isActive() &&
          movie->getShowtimeCount() == 1,
          "Co suat tuong lai: chan xoa va ngung phim");
    future.setStatus(ShowtimeStatus::CANCELLED);
    check(f.service.setMovieActive("MOV0001", false) && !movie->isActive() &&
          movie->getShowtimeCount() == 1,
          "Suat CANCELLED khong chan ngung phim");
    check(f.service.setMovieActive("MOV0001", false) &&
          f.service.setMovieActive("MOV0001", true) && movie->isActive(),
          "Dat lai trang thai va kich hoat lai");
    check(movie->unlinkShowtime(&future) && f.service.removeMovie("MOV0001"),
          "Unlink roi moi xoa phim");
}

void testFiltersAndSearch()
{
    cout << "\n===== TEST FILTER / SEARCH =====" << endl;
    string today = DateTime::getCurrentDateTime().substr(0, 10);
    string data = row("MOV0001", "Today Start", today, "2099-12-31") +
                  row("MOV0002", "Today End", "2020-01-01", today) +
                  row("MOV0003", "Future Movie", "2099-01-01", "2099-12-31") +
                  row("MOV0004", "Inactive Movie", today, "2099-12-31", false) +
                  row("MOV0005", "Old Movie", "2020-01-01", "2020-12-31");
    Fixture f(data);
    f.login();
    f.service.load();
    auto now = f.service.getNowShowingMovies();
    auto soon = f.service.getComingSoonMovies();
    check(now.size() == 2 && now[0]->getMovieId() == "MOV0001" &&
          now[1]->getMovieId() == "MOV0002", "Loc phim tai moc bat dau va ket thuc");
    check(soon.size() == 1 && soon[0]->getMovieId() == "MOV0003",
          "Loc sap chieu, loai inactive va het han");
    auto title = f.service.searchMoviesByTitle("mOvIe");
    check(title.size() == 3 && title[0]->getMovieId() == "MOV0003" &&
          title[1]->getMovieId() == "MOV0004" &&
          title[2]->getMovieId() == "MOV0005",
          "Tim chuoi con khong phan biet ASCII, gom inactive");
    check(f.service.searchMoviesByTitle("").size() == 5 &&
          f.service.searchMoviesByTitle("khong co").empty(),
          "Keyword rong va khong khop");
}

void testBeforeLoadAndEmptyResults()
{
    cout << "\n===== TEST BEFORE LOAD / EMPTY RESULTS =====" << endl;
    Fixture f;
    f.login();
    Movie values = makeMovie("MOV0001");
    check(!f.service.updateMovie(values) &&
          !f.service.setMovieActive("MOV0001", false) &&
          !f.service.removeMovie("MOV0001"),
          "Update, setActive, remove truoc load tra false");
    check(f.service.getNowShowingMovies().empty() &&
          f.service.getComingSoonMovies().empty() &&
          f.service.searchMoviesByTitle("x").empty(),
          "Loc va tim truoc load tra vector rong");
    f.service.load();
    check(!f.service.setMovieActive("MOV9999", false) &&
          f.service.getNowShowingMovies().empty() &&
          f.service.getComingSoonMovies().empty() &&
          f.service.searchMoviesByTitle("").empty(),
          "ID thieu va danh sach rong sau load");
}

int main()
{
    try
    {
        testInterfaceLoadSaveAndQueries();
        testPermissionOnly();
        testAddUpdateRemove();
        testShowtimeRules();
        testFiltersAndSearch();
        testBeforeLoadAndEmptyResults();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
