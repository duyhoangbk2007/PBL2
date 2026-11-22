#pragma once
#include "../business/BookingService.h"
#include <string>
#include <unordered_set>

namespace BookingAdapter {
enum class SeatStatus { Available, Sold, Unknown };
enum class SaleResult { NotImplemented };

struct SeatAvailability {
    bool verified = false;
    std::unordered_set<std::string> availableSeatIds;

    SeatStatus status(const std::string& seatId) const
    {
        if (!verified) { return SeatStatus::Unknown; }
        return availableSeatIds.count(seatId) ? SeatStatus::Available : SeatStatus::Sold;
    }
};

inline SeatAvailability loadAvailability(BookingService& booking, const std::string& showtimeId)
{
    SeatAvailability result;
    for (const Seat* seat : booking.getAvailableSeats(showtimeId)) {
        if (seat == nullptr) { return result; }
        result.availableSeatIds.insert(seat->getSeatId());
    }
    result.verified = true;
    return result;
}

inline SeatStatus seatStatus(const std::string& /*showtimeId*/, const std::string& /*seatId*/)
{
    return SeatStatus::Unknown;
}

inline SaleResult saleSupport()
{
    return SaleResult::NotImplemented;
}
}
