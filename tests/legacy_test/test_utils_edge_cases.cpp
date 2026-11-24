
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include "../src/utils/DateTime.h"
#include "../src/utils/IdGenerator.h"
#include "../src/utils/Validator.h"
#include "../src/utils/FileIO.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    if (condition)
    {
        passed++;
        cout << "[PASS] " << name << '\n';
    }
    else
    {
        failed++;
        cout << "[FAIL] " << name << '\n';
    }
}

void writeTestFile(const string& path, const string& content)
{
    ofstream file(path);
    if (!file.is_open())
    {
        throw runtime_error("Khong tao duoc file test");
    }
    file << content;
    file.close();

    if (!file)
    {
        throw runtime_error("Khong ghi duoc file test");
    }
}

void testDateTime()
{
    cout << "\n===== DATETIME =====\n";

    check(
        DateTime::addMinutes("2026-01-01T10:00:00", 90)
            == "2026-01-01T11:30:00",
        "Cong phut thong thuong"
    );

    check(
        DateTime::addMinutes("2026-01-31T23:45:00", 30)
            == "2026-02-01T00:15:00",
        "Qua ngay va thang"
    );

    check(
        DateTime::addMinutes("2026-12-31T23:50:00", 20)
            == "2027-01-01T00:10:00",
        "Qua nam"
    );

    check(
        DateTime::addMinutes("2024-02-28T23:30:00", 60)
            == "2024-02-29T00:30:00",
        "Nam nhuan"
    );

    check(
        DateTime::addMinutes("2025-02-28T23:30:00", 60)
            == "2025-03-01T00:30:00",
        "Nam khong nhuan"
    );

    bool threw = false;

    try
    {
        DateTime::addMinutes("2026-01-01T10:00:00", -1);
    }
    catch (const invalid_argument&)
    {
        threw = true;
    }

    check(threw, "Tu choi so phut am");

    check(
        DateTime::isBefore(
            "2026-01-01T10:00:00",
            "2026-01-01T11:00:00"
        ),
        "So sanh thoi gian"
    );

    check(
        DateTime::isOverlapping(
            "2026-01-01T10:00:00",
            "2026-01-01T12:00:00",
            "2026-01-01T12:10:00",
            "2026-01-01T14:00:00"
        ),
        "Trung lich trong 15 phut nghi"
    );

    check(
        !DateTime::isOverlapping(
            "2026-01-01T10:00:00",
            "2026-01-01T12:00:00",
            "2026-01-01T12:15:00",
            "2026-01-01T14:00:00"
        ),
        "Khong trung lich khi du 15 phut"
    );
}

void testIdGenerator()
{
    cout << "\n===== ID GENERATOR =====\n";

    const string path = "test_sequences_edge.txt";

    string id;
    string updatedSequences;

    writeTestFile(
        path,
        "MOV|1\n"
        "TKT|9999\n"
        "INV|10000\n"
    );

    IdGenerator::generateNextId(
        "MOV",
        path,
        id,
        updatedSequences
    );

    check(id == "MOV0001", "Tao ID dau tien");
    check(
        updatedSequences.find("MOV|2\n") != string::npos,
        "Tang bo dem MOV"
    );

    IdGenerator::generateNextId(
        "TKT",
        path,
        id,
        updatedSequences
    );

    check(id == "TKT9999", "Tao ID cuoi cung");
    check(
        updatedSequences.find("TKT|10000\n") != string::npos,
        "Bo dem dat 10000"
    );

    bool threw = false;

    try
    {
        IdGenerator::generateNextId(
            "INV",
            path,
            id,
            updatedSequences
        );
    }
    catch (const runtime_error&)
    {
        threw = true;
    }

    check(threw, "Tu choi khi het ID");
    check(
        id.empty() && updatedSequences.empty(),
        "Khong tra ket qua do dang"
    );

    writeTestFile(path, "MOV|1\nMOV|2\n");

    threw = false;

    try
    {
        IdGenerator::generateNextId(
            "MOV",
            path,
            id,
            updatedSequences
        );
    }
    catch (const runtime_error&)
    {
        threw = true;
    }

    check(threw, "Phat hien entity trung");

    writeTestFile(path, "MOV|abc\n");

    threw = false;

    try
    {
        IdGenerator::generateNextId(
            "MOV",
            path,
            id,
            updatedSequences
        );
    }
    catch (const runtime_error&)
    {
        threw = true;
    }

    check(threw, "Phat hien bo dem khong hop le");

    writeTestFile(path, "MOV|1\n");

    threw = false;

    try
    {
        IdGenerator::generateNextId(
            "ABC",
            path,
            id,
            updatedSequences
        );
    }
    catch (const runtime_error&)
    {
        threw = true;
    }

    check(threw, "Tu choi entity khong hop le");

    std::remove(path.c_str());
}

void testFileIO()
{
    cout << "\n===== FILE IO =====\n";

    const string path = "test_fileio_edge.txt";

    vector<string> original = {"A", "B", "C"};

    check(
        FileIO::writeLines(path, original),
        "Ghi file TXT"
    );

    vector<string> loaded;

    check(
        FileIO::readLines(path, loaded),
        "Doc file TXT"
    );

    check(
        loaded == original,
        "Noi dung khong thay doi"
    );

    vector<string> empty;

    check(
        FileIO::writeLines(path, empty),
        "Ghi file rong"
    );

    loaded.clear();

    check(
        FileIO::readLines(path, loaded) && loaded.empty(),
        "Doc file rong"
    );

    vector<string> unchanged = {"KEEP"};

    check(
        !FileIO::readLines(
            "file_khong_ton_tai_123456.txt",
            unchanged
        ),
        "Tu choi file khong ton tai"
    );

    check(
        unchanged.size() == 1 && unchanged[0] == "KEEP",
        "Khong thay doi output khi doc loi"
    );

    std::remove(path.c_str());
}

int main()
{
    try
    {
        testDateTime();
        testIdGenerator();
        testFileIO();
    }
    catch (const exception& e)
    {
        cerr << "TEST ERROR: " << e.what() << '\n';
        return 1;
    }

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "Total: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
