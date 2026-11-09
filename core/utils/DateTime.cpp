#include "DateTime.h"

namespace {

// Đọc một số có đúng length chữ số, bắt đầu từ vị trí start.
int docSo(const std::string& text, std::size_t start, std::size_t length) {
    int number = 0;

    for (std::size_t i = start; i < start + length; ++i) {
        // Chỉ chấp nhận các ký tự từ '0' đến '9'.
        if (text[i] < '0' || text[i] > '9') {
            return -1;
        }

        number = number * 10 + (text[i] - '0');
    }

    return number;
}

bool laNamNhuan(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    return year % 4 == 0;
}

int soNgayTrongThang(int year, int month) {
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && laNamNhuan(year)) {
        return 29;
    }

    return days[month - 1];
}

bool laNgayHopLe(const std::tm& value) {
    const int year = value.tm_year + 1900;
    const int month = value.tm_mon + 1;

    // Định dạng của dự án có đúng 4 chữ số cho năm: từ 0001 đến 9999.
    if (year < 1 || year > 9999) return false;
    if (month < 1 || month > 12) return false;
    if (value.tm_mday < 1 || value.tm_mday > soNgayTrongThang(year, month)) return false;

    return true;
}

void themHaiChuSo(std::string& text, int number) {
    text += static_cast<char>('0' + number / 10);
    text += static_cast<char>('0' + number % 10);
}

} // namespace

bool DateTime::chuoiThanhNgay(const std::string& text, std::tm& result) {
    // YYYY-MM-DD có đúng 10 ký tự.
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        return false;
    }

    const int year = docSo(text, 0, 4);
    const int month = docSo(text, 5, 2);
    const int day = docSo(text, 8, 2);

    if (year < 0 || month < 0 || day < 0) return false;

    std::tm parsed{};
    parsed.tm_year = year - 1900;
    parsed.tm_mon = month - 1;
    parsed.tm_mday = day;

    if (!laNgayHopLe(parsed)) return false;

    // Chỉ ghi kết quả sau khi đã kiểm tra xong, nên dữ liệu cũ được giữ lại nếu lỗi.
    result = parsed;
    return true;
}

bool DateTime::chuoiThanhNgayGio(const std::string& text, std::tm& result) {
    // YYYY-MM-DDTHH:mm:ss có đúng 19 ký tự.
    if (text.size() != 19 || text[10] != 'T' || text[13] != ':' || text[16] != ':') {
        return false;
    }

    std::tm parsed{};
    if (!chuoiThanhNgay(text.substr(0, 10), parsed)) return false;

    const int hour = docSo(text, 11, 2);
    const int minute = docSo(text, 14, 2);
    const int second = docSo(text, 17, 2);

    if (hour < 0 || minute < 0 || second < 0) return false;
    if (hour > 23 || minute > 59 || second > 59) return false;

    parsed.tm_hour = hour;
    parsed.tm_min = minute;
    parsed.tm_sec = second;

    result = parsed;
    return true;
}

std::string DateTime::ngayThanhChuoi(const std::tm& value) {
    if (!laNgayHopLe(value)) return "";

    const int year = value.tm_year + 1900;
    const int month = value.tm_mon + 1;
    std::string text;

    // Tách năm thành 4 chữ số để luôn giữ đúng định dạng.
    text += static_cast<char>('0' + year / 1000);
    text += static_cast<char>('0' + (year / 100) % 10);
    text += static_cast<char>('0' + (year / 10) % 10);
    text += static_cast<char>('0' + year % 10);
    text += '-';
    themHaiChuSo(text, month);
    text += '-';
    themHaiChuSo(text, value.tm_mday);

    return text;
}

std::string DateTime::ngayGioThanhChuoi(const std::tm& value) {
    if (value.tm_hour < 0 || value.tm_hour > 23 ||
        value.tm_min < 0 || value.tm_min > 59 ||
        value.tm_sec < 0 || value.tm_sec > 59) {
        return "";
    }

    std::string text = ngayThanhChuoi(value);
    if (text.empty()) return "";

    text += 'T';
    themHaiChuSo(text, value.tm_hour);
    text += ':';
    themHaiChuSo(text, value.tm_min);
    text += ':';
    themHaiChuSo(text, value.tm_sec);

    return text;
}
