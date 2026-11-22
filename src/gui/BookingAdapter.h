#pragma once
#include <string>

// TODO: BACKEND INTEGRATION — replace with the seat-status and sale transaction services.
namespace BookingAdapter {
enum class SeatStatus { Unknown };
enum class SaleResult { NotImplemented };

inline SeatStatus seatStatus(const std::string& /*showtimeId*/, const std::string& /*seatId*/)
{
    return SeatStatus::Unknown;
}

inline SaleResult saleSupport()
{
    return SaleResult::NotImplemented;
}
}
