#ifndef PBL2_DATE_TIME_H
#define PBL2_DATE_TIME_H

#include <string>

class DateTime {
public:
    static constexpr int BREAK_MINUTES = 15;
    static std::string getCurrentDateTime();
    static std::string addMinutes(const std::string& dateTime, int minutes); // cộng thêm số phút vào 1 thời điểm (tính giờ kết thúc suất chiếu)
    static bool isBefore(const std::string& first, const std::string& second); // kiểm tra start có trước end hay k
    static bool isOverlapping(const std::string& start1, const std::string& end1, const std::string& start2, const std::string& end2);
};

#endif