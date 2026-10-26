#include "Seat.h"
using namespace std;
Seat::Seat(const string& roomId,
           const string& seatId,
           int row,
           int column,
           SeatType seatType)
    : roomId(roomId),
      seatId(seatId),
      row(row),
      column(column),
      seatType(seatType) 
{}

const string& Seat::getRoomId() const {
    return this->roomId;
}
const string& Seat::getSeatId() const {
    return this->seatId;
}
int Seat::getRow() const {
    return this->row;
}
int Seat::getColumn() const {
    return this->column;
}
SeatType Seat::getSeatType() const {
    return this->seatType;
}

void Seat::setSeatType(SeatType seatType) {
    this->seatType = seatType;
}

