
#include <iostream>
#include <string>
#include <stdexcept>

#include "utils/DateTime.h"
#include "utils/Validator.h"

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
    cout << "===== TEST VALIDATOR DATETIME =====" << endl;

    check(Validator::isValidDateTime("2026-12-01T18:30:00"),
          "Valid datetime");

    check(!Validator::isValidDateTime("2026-12-01 18:30:00"),
          "Missing T");

    check(!Validator::isValidDateTime("2026-02-30T18:30:00"),
          "Invalid date");

    check(!Validator::isValidDateTime("2026-12-01T25:00:00"),
          "Invalid hour");

    check(!Validator::isValidDateTime("2026-12-01T18:60:00"),
          "Invalid minute");

    cout << "\n===== TEST ADD MINUTES =====" << endl;

    check(DateTime::addMinutes("2026-12-01T18:00:00", 120)
          == "2026-12-01T20:00:00",
          "Add 120 minutes");

    check(DateTime::addMinutes("2026-12-01T23:30:00", 60)
          == "2026-12-02T00:30:00",
          "Cross midnight");

    check(DateTime::addMinutes("2026-12-31T23:30:00", 60)
          == "2027-01-01T00:30:00",
          "Cross year");

    check(DateTime::addMinutes("2026-12-01T18:00:00", 0)
          == "2026-12-01T18:00:00",
          "Add zero minutes");

    bool caught = false;
    try
    {
        DateTime::addMinutes("2026-12-01T18:00:00", -10);
    }
    catch (const invalid_argument&)
    {
        caught = true;
    }
    check(caught, "Reject negative minutes");

    cout << "\n===== TEST IS BEFORE =====" << endl;

    check(DateTime::isBefore(
        "2026-12-01T18:00:00",
        "2026-12-01T20:00:00"),
        "Earlier time");

    check(!DateTime::isBefore(
        "2026-12-01T20:00:00",
        "2026-12-01T18:00:00"),
        "Later time");

    check(!DateTime::isBefore(
        "2026-12-01T18:00:00",
        "2026-12-01T18:00:00"),
        "Equal times");

    check(DateTime::isBefore(
        "2026-12-01T23:30:00",
        "2026-12-02T00:30:00"),
        "Compare across days");

    cout << "\n===== TEST OVERLAPPING =====" << endl;

    string start1 = "2026-12-01T18:00:00";
    string end1   = "2026-12-01T20:00:00";

    check(DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T19:00:00",
        "2026-12-01T21:00:00"),
        "Overlapping showtimes");

    check(DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T20:10:00",
        "2026-12-01T22:10:00"),
        "Conflict during break");

    check(!DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T20:15:00",
        "2026-12-01T22:15:00"),
        "Exactly 15 minutes break");

    check(!DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T20:30:00",
        "2026-12-01T22:30:00"),
        "No conflict");

    check(DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T16:00:00",
        "2026-12-01T18:05:00"),
        "Earlier showtime conflict");

    check(!DateTime::isOverlapping(
        start1, end1,
        "2026-12-01T15:00:00",
        "2026-12-01T17:45:00"),
        "Earlier showtime with enough break");

    cout << "\n===== TEST INVALID OVERLAP =====" << endl;

    caught = false;
    try
    {
        DateTime::isOverlapping(
            "2026-12-01T20:00:00",
            "2026-12-01T18:00:00",
            start1, end1);
    }
    catch (const invalid_argument&)
    {
        caught = true;
    }
    check(caught, "Reject end before start");

    caught = false;
    try
    {
        DateTime::isOverlapping(
            "invalid", end1,
            start1, end1);
    }
    catch (const invalid_argument&)
    {
        caught = true;
    }
    check(caught, "Reject invalid datetime");

    cout << "\n===== TEST SUMMARY =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;

    return failed == 0 ? 0 : 1;
}
