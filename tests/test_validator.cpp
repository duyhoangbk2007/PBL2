
#include <iostream>
#include <string>
#include "../src/utils/Validator.h"

using namespace std;

int passed = 0;
int failed = 0;

// Ham kiem tra ket qua test
void check(const string& testName, bool actual, bool expected)
{
    if (actual == expected)
    {
        cout << "[PASS] " << testName << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << testName
             << " | Expected: " << expected
             << " | Actual: " << actual << endl;
        failed++;
    }
}

// Test ham isValidText()
void testValidText()
{
    cout << "\n===== TEST isValidText =====\n";

    // Chuoi hop le
    check("Chuoi binh thuong",
          Validator::isValidText("Avatar"), true);

    check("Chuoi co dau cach",
          Validator::isValidText("Avengers Endgame"), true);

    check("Chuoi co dau cach dau cuoi",
          Validator::isValidText("  Avatar  "), true);

    check("Chuoi tieng Viet",
          Validator::isValidText("Phim hanh dong"), true);

    check("Chuoi co chu so",
          Validator::isValidText("Avatar 2"), true);

    check("Chuoi co ky tu dac biet",
          Validator::isValidText("Spider-Man: No Way Home"), true);

    check("Chuoi co dau cau",
          Validator::isValidText("Hello, World!"), true);

    // Chuoi khong hop le
    check("Chuoi rong",
          Validator::isValidText(""), false);

    check("Chi co dau cach",
          Validator::isValidText("     "), false);

    check("Chi co mot dau cach",
          Validator::isValidText(" "), false);

    check("Chua dau phan cach |",
          Validator::isValidText("Avatar|Titanic"), false);

    check("Chi co dau |",
          Validator::isValidText("|"), false);

    check("Chua xuong dong \\n",
          Validator::isValidText("Avatar\nTitanic"), false);

    check("Chua ky tu \\r",
          Validator::isValidText("Avatar\rTitanic"), false);

    check("Chua ky tu tab \\t",
          Validator::isValidText("Avatar\tTitanic"), false);

    check("Chi co tab",
          Validator::isValidText("\t\t"), false);

    check("Chua ky tu NULL",
          Validator::isValidText(string("Avatar\0Titanic", 14)), false);

    check("Chua ky tu DEL",
          Validator::isValidText(string("Avatar") + char(127)), false);
}

// Test ham isValidDate()
void testValidDate()
{
    cout << "\n===== TEST isValidDate =====\n";

    // Ngay hop le
    check("Ngay binh thuong",
          Validator::isValidDate("2026-11-12"), true);

    check("Ngay dau nam",
          Validator::isValidDate("2026-01-01"), true);

    check("Ngay cuoi nam",
          Validator::isValidDate("2026-12-31"), true);

    check("Ngay 30 thang 4",
          Validator::isValidDate("2026-04-30"), true);

    check("Ngay 28 thang 2 nam thuong",
          Validator::isValidDate("2025-02-28"), true);

    check("Ngay 29 thang 2 nam nhuan",
          Validator::isValidDate("2024-02-29"), true);

    check("Nam nhuan chia het cho 400",
          Validator::isValidDate("2000-02-29"), true);

    check("Nam 0001",
          Validator::isValidDate("0001-01-01"), true);

    // Ngay khong hop le
    check("Chuoi ngay rong",
          Validator::isValidDate(""), false);

    check("Sai dinh dang DD-MM-YYYY",
          Validator::isValidDate("12-11-2026"), false);

    check("Thieu so 0 o thang",
          Validator::isValidDate("2026-1-12"), false);

    check("Thieu so 0 o ngay",
          Validator::isValidDate("2026-11-2"), false);

    check("Dung dau / thay vi -",
          Validator::isValidDate("2026/11/12"), false);

    check("Chua chu cai",
          Validator::isValidDate("202A-11-12"), false);

    check("Chua dau cach",
          Validator::isValidDate("2026- 1-12"), false);

    check("Nam bang 0",
          Validator::isValidDate("0000-01-01"), false);

    check("Thang bang 0",
          Validator::isValidDate("2026-00-15"), false);

    check("Thang lon hon 12",
          Validator::isValidDate("2026-13-15"), false);

    check("Ngay bang 0",
          Validator::isValidDate("2026-01-00"), false);

    check("Ngay lon hon 31",
          Validator::isValidDate("2026-01-32"), false);

    check("Thang 4 co 31 ngay",
          Validator::isValidDate("2026-04-31"), false);

    check("Thang 6 co 31 ngay",
          Validator::isValidDate("2026-06-31"), false);

    check("Thang 2 co 30 ngay",
          Validator::isValidDate("2024-02-30"), false);

    check("Nam thuong co 29/2",
          Validator::isValidDate("2025-02-29"), false);

    check("Nam 1900 khong nhuan",
          Validator::isValidDate("1900-02-29"), false);

    check("Nam 2100 khong nhuan",
          Validator::isValidDate("2100-02-29"), false);

    check("Thua ky tu",
          Validator::isValidDate("2026-11-12abc"), false);

    check("Thieu ky tu",
          Validator::isValidDate("2026-11-1"), false);
}

int main()
{
    cout << "====================================\n";
    cout << "       VALIDATOR UNIT TESTS\n";
    cout << "====================================\n";

    testValidText();
    testValidDate();

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
