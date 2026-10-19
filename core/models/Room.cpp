#include "Room.h"
using namespace std;

Room::Room()
    : roomId(""),
      name(""),
      rowCount(0),
      columnCount(0),
      active(false)
{}

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
