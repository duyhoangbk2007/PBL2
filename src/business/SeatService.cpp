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
    authService.requireAdmin();
    return seatRepo.add(seat);
}

bool SeatService::updateSeat(const Seat& newValues)
{
    authService.requireAdmin();
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
    authService.requireAdmin();
    return seatRepo.removeById(roomId, seatId);
}

const Seat* SeatService::findSeatById(const string& roomId, const string& seatId)
{
    authService.requireLogin();
    return seatRepo.findById(roomId, seatId);
}

const Seat* SeatService::getSeatAt(int index)
{
    authService.requireLogin();
    return seatRepo.getAt(index);
}

int SeatService::getSeatCount()
{
    authService.requireLogin();
    return seatRepo.count();
}

