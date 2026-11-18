#include "SeatService.h"

using namespace std;

SeatService::SeatService(SeatRepository& seatRepo, AuthService& authService)
    : seatRepo(seatRepo), authService(authService)
{}

bool SeatService::load()
{
    return seatRepo.load();
}

bool SeatService::save()
{
    return seatRepo.save();
}

bool SeatService::addSeat(Seat* seat)
{
    authService.requirePermission("SEAT_ADD");
    return seatRepo.add(seat);
}

bool SeatService::updateSeat(const Seat& newValues)
{
    authService.requirePermission("SEAT_UPDATE");
    if (!seatRepo.isLoaded())
    {
        return false;
    }
    Seat* seat = seatRepo.findById(newValues.getRoomId(), newValues.getSeatId());
    if (seat == nullptr)
    {
        return false;
    }
    return seatRepo.update(newValues);
}

bool SeatService::removeSeat(const string& roomId, const string& seatId)
{
    authService.requirePermission("SEAT_DELETE");
    return seatRepo.removeById(roomId, seatId);
}

const Seat* SeatService::findSeatById(const string& roomId, const string& seatId)
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.findById(roomId, seatId);
}

const Seat* SeatService::getSeatAt(int index)
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.getAt(index);
}

int SeatService::getSeatCount()
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.count();
}

