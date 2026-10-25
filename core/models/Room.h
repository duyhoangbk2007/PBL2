#ifndef PBL2_CORE_MODELS_ROOM_H
#define PBL2_CORE_MODELS_ROOM_H

#include <string>
#include "../data_structures/DoublyLinkedList.h"

class Seat;
class Room {
private:
    DoublyLinkedList<Seat*> seats;  
    std::string roomId;
    std::string name;
    int rowCount;
    int columnCount;
    bool active;

public:
    Room();
    Room(const std::string& roomId, const std::string& name, int rowCount, int columnCount, bool active);

    const std::string& getRoomId() const;
    const std::string& getName() const;
    int getRowCount() const;
    int getColumnCount() const;
    bool isActive() const;

    void setName(const std::string& name);
    void setRowCount(int rowCount);
    void setColumnCount(int columnCount);
    void setActive(bool active);

    bool linkSeat(Seat* seat);
    Seat* findSeat(const std::string& seatId);
    int getSeatCount() const;
};

#endif
