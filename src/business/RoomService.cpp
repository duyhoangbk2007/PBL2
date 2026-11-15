#include "RoomService.h"

using namespace std;

RoomService::RoomService(RoomRepository& roomRepo, AuthService& authService)
    : roomRepo(roomRepo), authService(authService)
{}

bool RoomService::load()
{
    return roomRepo.load();
}

bool RoomService::save()
{
    return roomRepo.save();
}

bool RoomService::addRoom(Room* room)
{
    authService.requireAdmin();
    return roomRepo.add(room);
}

bool RoomService::updateRoom(const Room& newValues)
{
    authService.requireAdmin();
    if (!roomRepo.isLoaded())
    {
        return false;
    }
    Room* room = roomRepo.findById(newValues.getRoomId());
    if (room == nullptr)
    {
        return false;
    }
    // Khong cho doi trang thai qua ham update
    if (room->isActive() != newValues.isActive())
    {
        return false;
    }
    bool sizeChanged = room->getRowCount() != newValues.getRowCount() || room->getColumnCount() != newValues.getColumnCount();
    // Khong cho doi kich thuoc neu da co suat chieu
    if (sizeChanged && room->getShowtimeCount() > 0)
    {
        return false;
    }
    return roomRepo.update(newValues);
}

bool RoomService::removeRoom(const string& roomId)
{
    authService.requireAdmin();
    return roomRepo.removeById(roomId);
}

const Room* RoomService::findRoomById(const string& roomId)
{
    authService.requireLogin();
    return roomRepo.findById(roomId);
}

const Room* RoomService::getRoomAt(int index)
{
    authService.requireLogin();
    return roomRepo.getAt(index);
}

int RoomService::getRoomCount()
{
    authService.requireLogin();
    return roomRepo.count();
}



