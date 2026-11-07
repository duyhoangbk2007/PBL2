#ifndef PBL2_DATE_TIME_H
#define PBL2_DATE_TIME_H

#include <ctime>
#include <string>

// Ngày giờ theo giờ Việt Nam (UTC+07:00), không tự đổi múi giờ.
class DateTime {
public:
    static bool parseDate(const std::string& text, std::tm& result);
    static bool parseDateTime(const std::string& text, std::tm& result);
    // Trả chuỗi rỗng nếu dữ liệu không hợp lệ.
    static std::string formatDate(const std::tm& value);
    static std::string formatDateTime(const std::tm& value);
};

#endif
