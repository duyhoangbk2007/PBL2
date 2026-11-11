#include "Validator.h"

using namespace std;

// Kiem tra chuoi hop le de luu vao file TXT
bool Validator::isValidText(const string& text)
{
    bool hasContent = false;
    for (unsigned char c : text)
    {
        // Khong cho phep dau | va ky tu dieu khien
        if (c == '|' || c < 32 || c == 127)
        {
            return false;
        }
        // Kiem tra chuoi co noi dung hay khong
        if (c != ' ')
        {
            hasContent = true;
        }
    }
    return hasContent;
}

// Kiem tra ngay YYYY-MM-DD, bao gom nam nhuan
bool Validator::isValidDate(const string& date)
{
    if (date.length() != 10 || date[4] != '-' || date[7] != '-')
    {
        return false;
    }
    // Kiem tra cac vi tri con lai la chu so
    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
        {
            continue;
        }
        if (date[i] < '0' || date[i] > '9')
        {
            return false;
        }
    }

    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8, 2));

    if (year < 1 || month < 1 || month > 12)
    {
        return false;
    }
    int daysInMonth[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    bool leapYear = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);

    if (leapYear)
    {
        daysInMonth[1] = 29;
    }
    return day >= 1 && day <= daysInMonth[month - 1];
}
