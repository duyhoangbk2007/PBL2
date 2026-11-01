#include "Room.h"
#include "Seat.h"
#include "Showtime.h"
using namespace std;

Room::Room(const string& roomId,
           const string& name,
           int rowCount,
           int columnCount,
           bool active)
    : roomId(roomId),
      name(name),
      rowCount(rowCount),
      columnCount(columnCount),
      active(active)
{}

const string& Room::getRoomId() const {
    return this->roomId;
}

const string& Room::getName() const {
    return this->name;
}

int Room::getRowCount() const {
    return this->rowCount;
}

int Room::getColumnCount() const {
    return this->columnCount;
}

bool Room::isActive() const {
    return this->active;
}

void Room::setName(const string& name) {
    this->name = name;
}

void Room::setRowCount(int rowCount) {
    this->rowCount = rowCount;
}

void Room::setColumnCount(int columnCount) {
    this->columnCount = columnCount;
}

void Room::setActive(bool active) {
    this->active = active;
}

bool Room::linkSeat(Seat* seat) {
    if (seat == nullptr) return false;
    if (seat->getRoomId() != this->roomId) return false;
    if (seat->getRoom() != nullptr) return false;
    if (seat->getRow() < 1 || seat->getRow() > this->rowCount || seat->getColumn() < 1 || seat->getColumn() > this->columnCount)
        return false;

    int n = this->seats.nodeCount();
    for (int i = 0; i < n; i++)
    {
        Seat* current = this->seats.getAt(i)->data;

        if (current->getSeatId() == seat->getSeatId()) return false;
        if (current->getRow() == seat->getRow() && current->getColumn() == seat->getColumn())
            return false;
    }   
    this->seats.pushBack(seat);
    seat->room = this;
    return true;
}

Seat* Room::findSeat(const string& seatId) {
    int n = this->seats.nodeCount();

    for (int i = 0; i < n; i++)
    {
        Seat* seat = this->seats.getAt(i)->data;
        if (seat->getSeatId() == seatId) return seat;
    }
    return nullptr;
}

int Room::getSeatCount() const {
    return this->seats.nodeCount();
}

bool Room::linkShowtime(Showtime* showtime) {
    if (showtime == nullptr) return false;
    if (showtime->getRoomId() != this->roomId)
        return false;
    if (showtime->getRoom() != nullptr)
        return false;

    int n = this->showtimes.nodeCount();
    for (int i = 0; i < n; i++) {
        Showtime* current = this->showtimes.getAt(i)->data;
        if (current->getShowtimeId() == showtime->getShowtimeId())
            return false;
    }

    this->showtimes.pushBack(showtime);
    showtime->room = this;

    return true;
}
Showtime* Room::findShowtime(const std::string& showtimeId) {
    int n = this->showtimes.nodeCount();
    for (int i = 0; i < n; i++) {
        Showtime* current = this->showtimes.getAt(i)->data;

        if (current->getShowtimeId() == showtimeId)
            return current;
    }
    return nullptr;
}

int Room::getShowtimeCount() const {
    return this->showtimes.nodeCount();
}
