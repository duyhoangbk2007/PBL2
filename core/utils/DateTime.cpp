#include "DateTime.h"

#include <iomanip>
#include <locale>
#include <sstream>

namespace {
bool matches(const std::string& text, const std::string& pattern) {
    if (text.size() != pattern.size()) return false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (pattern[i] == '*') {
            if (text[i] < '0' || text[i] > '9') return false;
        } else if (text[i] != pattern[i]) return false;
    }
    return true;
}

bool validDate(const std::tm& value) {
    if (value.tm_year < -1899 || value.tm_year > 8099 ||
        value.tm_mon < 0 || value.tm_mon > 11) return false;
    const int year = value.tm_year + 1900;
    const bool leapYear = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    const int days[] = {31, leapYear ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return value.tm_mday >= 1 && value.tm_mday <= days[value.tm_mon];
}
}

bool DateTime::parseDate(const std::string& text, std::tm& result) {
    if (!matches(text, "****-**-**")) return false;
    std::tm value{};
    value.tm_year = std::stoi(text.substr(0, 4)) - 1900;
    value.tm_mon = std::stoi(text.substr(5, 2)) - 1;
    value.tm_mday = std::stoi(text.substr(8, 2));
    if (!validDate(value)) return false;
    result = value;
    return true;
}

bool DateTime::parseDateTime(const std::string& text, std::tm& result) {
    if (!matches(text, "****-**-**T**:**:**")) return false;
    std::tm value{};
    if (!parseDate(text.substr(0, 10), value)) return false;
    value.tm_hour = std::stoi(text.substr(11, 2));
    value.tm_min = std::stoi(text.substr(14, 2));
    value.tm_sec = std::stoi(text.substr(17, 2));
    if (value.tm_hour > 23 || value.tm_min > 59 || value.tm_sec > 59) return false;
    result = value;
    return true;
}

std::string DateTime::formatDate(const std::tm& value) {
    if (!validDate(value)) return "";
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::setfill('0') << std::setw(4) << value.tm_year + 1900 << '-'
           << std::setw(2) << value.tm_mon + 1 << '-' << std::setw(2) << value.tm_mday;
    return output.str();
}

std::string DateTime::formatDateTime(const std::tm& value) {
    const std::string date = formatDate(value);
    if (date.empty() || value.tm_hour < 0 || value.tm_hour > 23 ||
        value.tm_min < 0 || value.tm_min > 59 || value.tm_sec < 0 || value.tm_sec > 59) return "";
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << date << 'T' << std::setfill('0') << std::setw(2) << value.tm_hour << ':'
           << std::setw(2) << value.tm_min << ':' << std::setw(2) << value.tm_sec;
    return output.str();
}
