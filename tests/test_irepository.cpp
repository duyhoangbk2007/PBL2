#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <type_traits>

#include "../src/data_access/IRepository.h"
#include "../src/data_access/MovieRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_irepository_movies.txt";

// Kiem tra ket qua test
void check(bool condition, const string& testName)
{
    if (condition)
    {
        cout << "[PASS] " << testName << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << testName << endl;
        failed++;
    }
}

// Tao file du lieu test
void createTestFile()
{
    ofstream file(TEST_FILE);

    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }

    file << "MOV0001|Interstellar|Sci-Fi|169|13|2026-11-01|2026-12-01|1\n";
    file << "MOV0002|Inception|Action|148|13|2026-11-01|2026-12-01|1\n";
}

// Tao doi tuong Movie dung de test
Movie* createMovie(const string& movieId,
                   const string& title,
                   int durationMinutes = 120)
{
    return new Movie(
        movieId,
        title,
        "Action",
        durationMinutes,
        13,
        "2026-11-01",
        "2026-12-01",
        true
    );
}

// Kiem tra cau truc interface
void testInterface()
{
    cout << "\n===== TEST INTERFACE =====" << endl;

    check(is_abstract<IRepository<Movie>>::value,
          "IRepository la lop truu tuong");

    check(is_base_of<IRepository<Movie>, MovieRepository>::value,
          "MovieRepository ke thua IRepository");

    check(has_virtual_destructor<IRepository<Movie>>::value,
          "IRepository co destructor ao");

    check(!is_abstract<MovieRepository>::value,
          "MovieRepository trien khai day du interface");
}

// Kiem tra trang thai truoc khi load
void testBeforeLoad(IRepository<Movie>& repo)
{
    cout << "\n===== TEST BEFORE LOAD =====" << endl;

    check(!repo.isLoaded(),
          "Repository chua load");

    check(repo.count() == 0,
          "Danh sach ban dau rong");

    check(repo.findById("MOV0001") == nullptr,
          "Find khi chua load");

    check(repo.getAt(0) == nullptr,
          "GetAt khi chua load");

    check(!repo.save(),
          "Khong save khi chua load");

    Movie* movie = createMovie("MOV0003", "Avatar");

    bool result = repo.add(movie);

    check(!result,
          "Khong add khi chua load");

    if (!result)
    {
        delete movie;
    }

    Movie newValues(
        "MOV0001",
        "Updated Movie",
        "Action",
        120,
        13,
        "2026-11-01",
        "2026-12-01",
        true
    );

    check(!repo.update(newValues),
          "Khong update khi chua load");

    check(!repo.removeById("MOV0001"),
          "Khong remove khi chua load");
}

// Kiem tra load
void testLoad(IRepository<Movie>& repo)
{
    cout << "\n===== TEST LOAD =====" << endl;

    check(repo.load(),
          "Load du lieu thanh cong");

    check(repo.isLoaded(),
          "Trang thai loaded sau load");

    check(repo.count() == 2,
          "Load dung so luong phim");

    check(!repo.load(),
          "Khong load lai Repository");

    check(repo.count() == 2,
          "Du lieu khong doi sau load lai");
}

// Kiem tra findById, getAt va count
void testQuery(IRepository<Movie>& repo)
{
    cout << "\n===== TEST QUERY =====" << endl;

    Movie* movie1 = repo.findById("MOV0001");
    Movie* movie2 = repo.findById("MOV0002");

    check(movie1 != nullptr,
          "Tim thay MOV0001");

    check(movie2 != nullptr,
          "Tim thay MOV0002");

    check(movie1 != nullptr &&
          movie1->getTitle() == "Interstellar",
          "Du lieu MOV0001 chinh xac");

    check(movie2 != nullptr &&
          movie2->getTitle() == "Inception",
          "Du lieu MOV0002 chinh xac");

    check(repo.findById("MOV9999") == nullptr,
          "Khong tim thay ID khong ton tai");

    check(repo.findById("") == nullptr,
          "Tim kiem ID rong");

    check(movie1 != nullptr && repo.getAt(0) == movie1,
          "GetAt vi tri 0");

    check(movie2 != nullptr && repo.getAt(1) == movie2,
          "GetAt vi tri 1");

    check(repo.getAt(-1) == nullptr,
          "GetAt vi tri am");

    check(repo.getAt(repo.count()) == nullptr,
          "GetAt vuot gioi han");

    check(repo.count() == 2,
          "Count chinh xac");
}

// Kiem tra add
void testAdd(IRepository<Movie>& repo)
{
    cout << "\n===== TEST ADD =====" << endl;

    // Them phim hop le
    Movie* movie = createMovie("MOV0003", "Avatar");

    bool result = repo.add(movie);

    check(result,
          "Them phim hop le");

    if (!result)
    {
        delete movie;
    }

    check(repo.count() == 3,
          "Count sau khi them");

    check(result && repo.findById("MOV0003") == movie,
          "Repository quan ly dung con tro");

    // Them nullptr
    check(!repo.add(nullptr),
          "Khong them nullptr");

    // Them phim trung ID
    Movie* duplicate = createMovie("MOV0001", "Duplicate");

    result = repo.add(duplicate);

    check(!result,
          "Khong them phim trung ID");

    if (!result)
    {
        delete duplicate;
    }

    // Them phim sai ID
    Movie* invalidId = createMovie("ROM0001", "Invalid ID");

    result = repo.add(invalidId);

    check(!result,
          "Khong them phim sai tien to ID");

    if (!result)
    {
        delete invalidId;
    }

    // Them phim sai thoi luong
    Movie* invalidDuration = createMovie(
        "MOV0004",
        "Invalid Duration",
        -10
    );

    result = repo.add(invalidDuration);

    check(!result,
          "Khong them phim co thoi luong am");

    if (!result)
    {
        delete invalidDuration;
    }

    // Them lai con tro da duoc quan ly
    Movie* existing = repo.findById("MOV0001");

    check(existing != nullptr && !repo.add(existing),
          "Khong them lai con tro da quan ly");

    check(repo.count() == 3,
          "Count khong doi sau cac lan add loi");
}

// Kiem tra update
void testUpdate(IRepository<Movie>& repo)
{
    cout << "\n===== TEST UPDATE =====" << endl;

    Movie* oldMovie = repo.findById("MOV0001");

    Movie newValues(
        "MOV0001",
        "Interstellar Updated",
        "Adventure",
        170,
        16,
        "2026-11-02",
        "2026-12-15",
        false
    );

    check(repo.update(newValues),
          "Cap nhat phim hop le");

    Movie* updatedMovie = repo.findById("MOV0001");

    check(updatedMovie != nullptr,
          "Tim thay phim sau update");

    check(updatedMovie != nullptr &&
          updatedMovie == oldMovie,
          "Giu nguyen dia chi doi tuong");

    check(updatedMovie != nullptr &&
          updatedMovie->getMovieId() == "MOV0001",
          "Giu nguyen ID");

    check(updatedMovie != nullptr &&
          updatedMovie->getTitle() == "Interstellar Updated",
          "Cap nhat title");

    check(updatedMovie != nullptr &&
          updatedMovie->getGenre() == "Adventure",
          "Cap nhat genre");

    check(updatedMovie != nullptr &&
          updatedMovie->getDurationMinutes() == 170,
          "Cap nhat durationMinutes");

    check(updatedMovie != nullptr &&
          updatedMovie->getMinAge() == 16,
          "Cap nhat minAge");

    check(updatedMovie != nullptr &&
          updatedMovie->getReleaseDate() == "2026-11-02",
          "Cap nhat releaseDate");

    check(updatedMovie != nullptr &&
          updatedMovie->getEndDate() == "2026-12-15",
          "Cap nhat endDate");

    check(updatedMovie != nullptr &&
          !updatedMovie->isActive(),
          "Cap nhat active");

    // Update phim khong ton tai
    Movie notFound(
        "MOV9999",
        "Not Found",
        "Action",
        120,
        13,
        "2026-11-01",
        "2026-12-01",
        true
    );

    check(!repo.update(notFound),
          "Khong update phim khong ton tai");

    // Update du lieu khong hop le
    Movie invalid(
        "MOV0001",
        "Invalid Movie",
        "Action",
        -100,
        13,
        "2026-11-01",
        "2026-12-01",
        true
    );

    check(!repo.update(invalid),
          "Khong update du lieu khong hop le");

    check(updatedMovie != nullptr &&
          updatedMovie->getTitle() == "Interstellar Updated" &&
          updatedMovie->getDurationMinutes() == 170,
          "Du lieu khong doi sau update loi");

    check(repo.count() == 3,
          "Count khong doi sau update");
}

// Kiem tra removeById
void testRemove(IRepository<Movie>& repo)
{
    cout << "\n===== TEST REMOVE =====" << endl;

    check(repo.removeById("MOV0002"),
          "Xoa phim hop le");

    check(repo.findById("MOV0002") == nullptr,
          "Khong tim thay phim da xoa");

    check(repo.count() == 2,
          "Count sau khi xoa");

    check(!repo.removeById("MOV0002"),
          "Khong xoa lai phim da xoa");

    check(!repo.removeById("MOV9999"),
          "Khong xoa phim khong ton tai");

    check(!repo.removeById(""),
          "Khong xoa ID rong");

    check(repo.count() == 2,
          "Count khong doi sau remove loi");
}

// Kiem tra save va load lai
void testSave(IRepository<Movie>& repo)
{
    cout << "\n===== TEST SAVE =====" << endl;

    check(repo.save(),
          "Save du lieu thanh cong");

    MovieRepository newMovieRepo(TEST_FILE);

    IRepository<Movie>& newRepo = newMovieRepo;

    check(newRepo.load(),
          "Load lai du lieu da save");

    check(newRepo.count() == 2,
          "So luong phim sau load lai");

    Movie* movie = newRepo.findById("MOV0001");

    check(movie != nullptr,
          "Tim thay phim da update");

    check(movie != nullptr &&
          movie->getTitle() == "Interstellar Updated",
          "Title duoc luu chinh xac");

    check(movie != nullptr &&
          movie->getGenre() == "Adventure",
          "Genre duoc luu chinh xac");

    check(movie != nullptr &&
          movie->getDurationMinutes() == 170,
          "Duration duoc luu chinh xac");

    check(movie != nullptr &&
          movie->getMinAge() == 16,
          "MinAge duoc luu chinh xac");

    check(movie != nullptr &&
          movie->getReleaseDate() == "2026-11-02",
          "ReleaseDate duoc luu chinh xac");

    check(movie != nullptr &&
          movie->getEndDate() == "2026-12-15",
          "EndDate duoc luu chinh xac");

    check(movie != nullptr && !movie->isActive(),
          "Active duoc luu chinh xac");

    check(newRepo.findById("MOV0002") == nullptr,
          "Phim da xoa khong con trong file");

    check(newRepo.findById("MOV0003") != nullptr,
          "Phim moi duoc luu vao file");
}

// Kiem tra da hinh qua con tro interface
void testPolymorphism()
{
    cout << "\n===== TEST POLYMORPHISM =====" << endl;

    IRepository<Movie>* repo = new MovieRepository(TEST_FILE);

    check(repo->load(),
          "Load qua con tro interface");

    check(repo->isLoaded(),
          "IsLoaded qua con tro interface");

    check(repo->count() == 2,
          "Count qua con tro interface");

    check(repo->findById("MOV0001") != nullptr,
          "FindById qua con tro interface");

    delete repo;

    check(true,
          "Delete qua con tro interface khong bi crash");
}

// Kiem tra load du lieu loi
void testInvalidLoad()
{
    cout << "\n===== TEST INVALID LOAD =====" << endl;

    const string invalidFile = "test_irepository_invalid.txt";

    ofstream file(invalidFile);

    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test loi");
    }

    file << "MOV0001|Interstellar|Sci-Fi|169|13|2026-11-01|2026-12-01|1\n";
    file << "MOV0002|Invalid Movie|Action|-100|13|2026-11-01|2026-12-01|1\n";

    file.close();

    {
        MovieRepository movieRepo(invalidFile);

        IRepository<Movie>& repo = movieRepo;

        bool hasException = false;

        try
        {
            repo.load();
        }
        catch (const runtime_error&)
        {
            hasException = true;
        }

        check(hasException,
              "Load du lieu loi nem exception");

        check(!repo.isLoaded(),
              "Loaded van false sau load loi");

        check(repo.count() == 0,
              "Khong giu lai du lieu khi load loi");

        check(repo.findById("MOV0001") == nullptr,
              "Du lieu tam da duoc giai phong");
    }

    remove(invalidFile.c_str());
}

int main()
{
    try
    {
        createTestFile();

        testInterface();

        {
            MovieRepository movieRepo(TEST_FILE);

            IRepository<Movie>& repo = movieRepo;

            testBeforeLoad(repo);
            testLoad(repo);
            testQuery(repo);
            testAdd(repo);
            testUpdate(repo);
            testRemove(repo);
            testSave(repo);
        }

        testPolymorphism();
        testInvalidLoad();
    }
    catch (const exception& e)
    {
        failed++;

        cout << "\n[ERROR] " << e.what() << endl;
    }

    remove(TEST_FILE.c_str());
    remove("test_irepository_invalid.txt");

    cout << "\n===== SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}