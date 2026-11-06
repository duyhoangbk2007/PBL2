#include "Seat.h"
#include "Room.h"
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

Room* Seat::getRoom() const {
    return this->room;
}

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

bool Seat::linkRoom(Room* room) {
    if (room == nullptr)
        return false;

    if (room->getRoomId() != this->roomId)
        return false;

    return room->linkSeat(this);
}


