#ifndef PBL2_CORE_MODELS_ROOM_H
#define PBL2_CORE_MODELS_ROOM_H

#include <string>
#include "../../data_structures/DoublyLinkedList.h"
class Showtime;
class Seat;
class Room {
private:
    std::string roomId;
    std::string name;
    int rowCount;
    int columnCount;
    bool active;

    DoublyLinkedList<Seat*> seats;  
    DoublyLinkedList<Showtime*> showtimes;
public:

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
    bool linkShowtime(Showtime* showtime);
    Showtime* findShowtime(const std::string& showtimeId);
    int getShowtimeCount() const;
};

#endif
