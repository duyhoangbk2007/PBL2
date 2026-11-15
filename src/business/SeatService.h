#ifndef SEAT_SERVICE_H
#define SEAT_SERVICE_H

#include <string>
#include "../data_access/SeatRepository.h"
#include "AuthService.h"

class SeatService
{
private:
    SeatRepository& seatRepo;
    AuthService& authService;

public:
    SeatService(SeatRepository& seatRepo, AuthService& authService);

    bool load();
    bool save();

    bool addSeat(Seat* seat);
    bool updateSeat(const Seat& newValues);
    bool removeSeat(const std::string& roomId, const std::string& seatId);

    const Seat* findSeatById(const std::string& roomId, const std::string& seatId);
    const Seat* getSeatAt(int index);
    int getSeatCount();
};

#endif
