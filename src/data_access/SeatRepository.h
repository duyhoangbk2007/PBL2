#ifndef SEAT_REPOSITORY_H
#define SEAT_REPOSITORY_H

#include <string>
#include "Repository.h"
#include "../models/Seat.h"
#include "RoomRepository.h"

class SeatRepository
{
private:
    Repository<Seat> repo;
    RoomRepository& roomRepo;

    static std::string getSeatKey(Seat* seat);
    static Seat* stringToSeat(const std::string& line);
    static std::string seatToString(Seat* seat);
    static bool isValidSeat(const Seat* seat);

public:
    SeatRepository(const std::string& filePath, RoomRepository& roomRepo);

    bool load();
    bool save();

    bool add(Seat* seat);
    bool update(const Seat& newValues);
    bool removeById(const std::string& roomId, const std::string& seatId);

    Seat* findById(const std::string& roomId, const std::string& seatId);
    Seat* getAt(int index);
    int count();
    bool isLoaded();
};

#endif
