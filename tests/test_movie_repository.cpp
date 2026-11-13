
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "../src/data_access/MovieRepository.h"

using namespace std;

int passed = 0;
int failed = 0;

const string TEST_FILE = "test_movies_temp.txt";

// Kiem tra ket qua
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

// Ghi du lieu mau vao file
void writeTestFile(const string& content)
{
    ofstream file(TEST_FILE);
    file << content;
}

// Xoa file test
void removeTestFile()
{
    remove(TEST_FILE.c_str());
}

// Tao Movie de test
Movie* createMovie(
    const string& id,
    const string& title = "Avatar",
    int duration = 120
)
{
    return new Movie(
        id,
        title,
        "Action",
        duration,
        13,
        "2026-01-01",
        "2026-12-31",
        true
    );
}

void testMovieRepository()
{
    cout << "\n===== TEST MOVIE REPOSITORY =====\n";

    // Tao file rong de load
    writeTestFile("");

    {
        MovieRepository repo(TEST_FILE);

        check("Chua load",
              repo.isLoaded(), false);

        check("Load file rong",
              repo.load(), true);

        check("Da load",
              repo.isLoaded(), true);

        check("Danh sach ban dau rong",
              repo.count() == 0, true);

        // Them phim
        Movie* movie1 = createMovie("MOV0001");

        check("Them phim hop le",
              repo.add(movie1), true);

        check("So luong sau khi them",
              repo.count() == 1, true);

        // Tim phim
        Movie* found = repo.findById("MOV0001");

        check("Tim phim ton tai",
              found != nullptr, true);

        check("Tim phim khong ton tai",
              repo.findById("MOV9999") == nullptr, true);

        check("Lay phim vi tri 0",
              repo.getAt(0) == movie1, true);

        check("Lay phim ngoai pham vi",
              repo.getAt(100) == nullptr, true);

        // Khong them lai cung con tro
        check("Them lai cung con tro",
              repo.add(movie1), false);

        // Trung ID
        Movie* duplicate = createMovie("MOV0001", "Titanic");

        bool addedDuplicate = repo.add(duplicate);

        check("Them trung ID",
              addedDuplicate, false);

        if (!addedDuplicate)
        {
            delete duplicate;
        }

        // ID sai
        Movie* invalidId = createMovie("MOV0000");

        bool addedInvalidId = repo.add(invalidId);

        check("Them ID sai",
              addedInvalidId, false);

        if (!addedInvalidId)
        {
            delete invalidId;
        }

        // Thoi luong sai
        Movie* invalidDuration = createMovie("MOV0002", "Test", 0);

        bool addedInvalidDuration = repo.add(invalidDuration);

        check("Them thoi luong sai",
              addedInvalidDuration, false);

        if (!addedInvalidDuration)
        {
            delete invalidDuration;
        }

        // Cap nhat phim
        Movie updated(
            "MOV0001",
            "Avatar Updated",
            "Sci-Fi",
            150,
            16,
            "2026-01-01",
            "2026-12-31",
            false
        );

        check("Cap nhat phim",
              repo.update(updated), true);

        Movie* afterUpdate = repo.findById("MOV0001");

        check("Ten phim sau update",
              afterUpdate != nullptr &&
              afterUpdate->getTitle() == "Avatar Updated",
              true);

        check("Thoi luong sau update",
              afterUpdate != nullptr &&
              afterUpdate->getDurationMinutes() == 150,
              true);

        check("Trang thai sau update",
              afterUpdate != nullptr &&
              !afterUpdate->isActive(),
              true);

        // Luu du lieu
        check("Save du lieu",
              repo.save(), true);

        // Xoa phim
        check("Xoa phim ton tai",
              repo.removeById("MOV0001"), true);

        check("So luong sau xoa",
              repo.count() == 0, true);

        check("Xoa phim khong ton tai",
              repo.removeById("MOV9999"), false);

        // Load lai tu file da save
        check("Load lai du lieu",
              repo.load(), true);

        check("So luong sau load lai",
              repo.count() == 1, true);

        Movie* reloaded = repo.findById("MOV0001");

        check("Ten phim sau load lai",
              reloaded != nullptr &&
              reloaded->getTitle() == "Avatar Updated",
              true);

        check("Trang thai sau load lai",
              reloaded != nullptr &&
              !reloaded->isActive(),
              true);
    }

    removeTestFile();
}

void testInvalidFiles()
{
    cout << "\n===== TEST INVALID FILES =====\n";

    removeTestFile();

    {
        MovieRepository repo(TEST_FILE);

        check("Load file khong ton tai",
              repo.load(), false);

        check("Van chua loaded",
              repo.isLoaded(), false);

        check("Save truoc load",
              repo.save(), false);
    }

    // File co ban ghi sai
    writeTestFile(
        "MOV0001|Avatar|Action|120|13|2026-01-01|2026-12-31|1\n"
        "MOV0002|Invalid|Action|0|13|2026-01-01|2026-12-31|1\n"
    );

    {
        MovieRepository repo(TEST_FILE);

        bool threw = false;

        try
        {
            repo.load();
        }
        catch (...)
        {
            threw = true;
        }

        check("Load du lieu sai nem exception",
              threw, true);

        check("Khong danh dau loaded khi loi",
              repo.isLoaded(), false);

        check("Khong giu du lieu doc do",
              repo.count() == 0, true);
    }

    // File trung ID
    writeTestFile(
        "MOV0001|Avatar|Action|120|13|2026-01-01|2026-12-31|1\n"
        "MOV0001|Titanic|Drama|150|13|2026-01-01|2026-12-31|1\n"
    );

    {
        MovieRepository repo(TEST_FILE);

        bool threw = false;

        try
        {
            repo.load();
        }
        catch (...)
        {
            threw = true;
        }

        check("Load file trung ID nem exception",
              threw, true);

        check("Khong nap du lieu trung ID",
              repo.count() == 0, true);
    }

    removeTestFile();
}

void testLoadPreservesOldData()
{
    cout << "\n===== TEST LOAD PRESERVES OLD DATA =====\n";

    writeTestFile(
        "MOV0001|Avatar|Action|120|13|2026-01-01|2026-12-31|1\n"
    );

    {
        MovieRepository repo(TEST_FILE);

        check("Load du lieu ban dau",
              repo.load(), true);

        check("Co mot phim ban dau",
              repo.count() == 1, true);

        // Thay file bang du lieu sai
        writeTestFile(
            "MOV0002|Invalid|Action|0|13|2026-01-01|2026-12-31|1\n"
        );

        bool threw = false;

        try
        {
            repo.load();
        }
        catch (...)
        {
            threw = true;
        }

        check("Load lai file sai nem exception",
              threw, true);

        check("Giu trang thai loaded",
              repo.isLoaded(), true);

        check("Giu so luong phim cu",
              repo.count() == 1, true);

        check("Giu phim cu",
              repo.findById("MOV0001") != nullptr, true);
    }

    removeTestFile();
}

int main()
{
    cout << "====================================\n";
    cout << "       MOVIE REPOSITORY TESTS\n";
    cout << "====================================\n";

    testMovieRepository();
    testInvalidFiles();
    testLoadPreservesOldData();

    cout << "\n====================================\n";
    cout << "             SUMMARY\n";
    cout << "====================================\n";

    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total : " << passed + failed << endl;

    if (failed == 0)
    {
        cout << "\nALL TESTS PASSED!\n";
        return 0;
    }

    cout << "\nSOME TESTS FAILED!\n";
    return 1;
}
