#include "DateTime.h"
#include "Validator.h"
#include <ctime>
#include <stdexcept>
using namespace std;


string DateTime::getCurrentDateTime() {
    time_t currentTime = time(nullptr);
    tm* local = localtime(&currentTime);

    if (local == nullptr) {
        throw runtime_error("Khong lay duoc thoi gian");
    }

    char buffer[20];

    if (strftime(buffer, sizeof(buffer),"%Y-%m-%dT%H:%M:%S", local) == 0) {
        throw runtime_error("Khong dinh dang duoc thoi gian");
    }

    return string(buffer);
}

string DateTime::addMinutes(const string& dateTime, int minutes)
{
    if (!Validator::isValidDateTime(dateTime) || minutes < 0)
    {
        throw invalid_argument("Thoi gian hoac so phut khong hop le");
    }

    tm timeInfo = {};

    // quy ước của tm_year là lưu số năm tính từ 1900 (2026 là 126)
    timeInfo.tm_year = stoi(dateTime.substr(0, 4)) - 1900;
    // quy ước của tm_mon là lưu từ tháng 0 đến tháng 11
    timeInfo.tm_mon = stoi(dateTime.substr(5, 2)) - 1;
    timeInfo.tm_mday = stoi(dateTime.substr(8, 2));
    timeInfo.tm_hour = stoi(dateTime.substr(11, 2));
    timeInfo.tm_min = stoi(dateTime.substr(14, 2));
    timeInfo.tm_sec = stoi(dateTime.substr(17, 2));
    timeInfo.tm_isdst = -1;

    time_t value = mktime(&timeInfo); // mktime() chuyển ngày giờ trong tm thành một giá trị time_t, thường lưu dưới dạng số giây

    if (value == static_cast<time_t>(-1))
    {
        throw runtime_error("Khong chuyen doi duoc thoi gian");
    }

    // Cong so phut vao thoi diem ban dau
    value += static_cast<time_t>(minutes) * 60;

    tm* result = localtime(&value); 

    if (result == nullptr)
    {
        throw runtime_error("Khong tinh duoc thoi gian ket thuc");
    }

    char buffer[20];
    if (strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", result) == 0) //chuyển ngày giờ thành chuỗi
    {
        throw runtime_error("Khong dinh dang duoc thoi gian");
    }
    return string(buffer);
}

bool DateTime::isBefore(const string& first, const string& second)
{
    if (!Validator::isValidDateTime(first) ||
        !Validator::isValidDateTime(second))
    {
        return false;
    }

    return first < second;
}

bool DateTime::isOverlapping(const string& start1, const string& end1, const string& start2, const string& end2)
{
    if (!Validator::isValidDateTime(start1) ||
        !Validator::isValidDateTime(end1) ||
        !Validator::isValidDateTime(start2) ||
        !Validator::isValidDateTime(end2))
    {
        throw invalid_argument("Thoi gian khong hop le");
    }

    if (!isBefore(start1, end1) || !isBefore(start2, end2))
    {
        throw invalid_argument("Thoi gian ket thuc phai sau bat dau");
    }

    string endWithBreak1 = addMinutes(end1, BREAK_MINUTES);
    string endWithBreak2 = addMinutes(end2, BREAK_MINUTES);
    return isBefore(start1, endWithBreak2) && isBefore(start2, endWithBreak1);
}

