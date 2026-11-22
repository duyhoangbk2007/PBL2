
#pragma once

#include "../business/BookingService.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace BookingAdapter {

enum class SeatStatus {
    Available,
    Sold,
    Unknown
};

struct SeatAvailability {
    std::unordered_set<std::string> availableIds;
    bool verified = false;

    SeatStatus status(const std::string& seatId) const
    {
        if (!verified) {
            return SeatStatus::Unknown;
        }

        return availableIds.count(seatId)
            ? SeatStatus::Available
            : SeatStatus::Sold;
    }
};

inline SeatAvailability loadAvailability(
    BookingService& service,
    const std::string& showtimeId)
{
    SeatAvailability result;

    // BookingService chịu trách nhiệm xác định ghế trống.
    const std::vector<Seat*> seats =
        service.getAvailableSeats(showtimeId);

    for (const Seat* seat : seats) {
        if (seat != nullptr) {
            result.availableIds.insert(
                seat->getSeatId());
        }
    }

    // Chỉ dùng khi các điều kiện nạp dữ liệu và
    // suất chiếu đã được kiểm tra ở phía gọi.
    result.verified = true;

    return result;
}

} // namespace BookingAdapter
