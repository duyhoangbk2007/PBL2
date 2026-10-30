#ifndef PBL2_CORE_MODELS_SEAT_H
#define PBL2_CORE_MODELS_SEAT_H

#include <string>
enum class SeatType {
    STANDARD,
    VIP,
    COUPLE
};

class Seat {
private:
    std::string roomId;
    std::string seatId;
    int row;
    int column;
    SeatType seatType;
public:
    Seat(const std::string& roomId, const std::string& seatId, int row, int column, SeatType seatType);

    const std::string& getRoomId() const;
    const std::string& getSeatId() const;
    int getRow() const;
    int getColumn() const;
    SeatType getSeatType() const;

    void setSeatType(SeatType seatType);
};

#endif
