#include "RoomService.h"
#include "../utils/IdGenerator.h"
#include <vector>
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
    authService.requirePermission("ROOM_ADD");

    if (!roomRepo.isLoaded() || room == nullptr)
    {
        return false;
    }

    vector<string> existingIds;

    for (int i = 0; i < roomRepo.count(); i++)
    {
        Room* current = roomRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getRoomId());
    }

    string roomId = IdGenerator::generateNextId("ROM", existingIds);

    Room* newRoom = new Room(roomId,
                             room->getName(),
                             room->getRowCount(),
                             room->getColumnCount(),
                             room->isActive());

    try
    {
        if (!roomRepo.add(newRoom))
        {
            delete newRoom;
            return false;
        }
    }
    catch (...)
    {
        delete newRoom;
        throw;
    }

    return true;
}


bool RoomService::updateRoom(const Room& newValues)
{
    authService.requirePermission("ROOM_UPDATE");
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
    authService.requirePermission("ROOM_DELETE");
    return roomRepo.removeById(roomId);
}

const Room* RoomService::findRoomById(const string& roomId)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.findById(roomId);
}

const Room* RoomService::getRoomAt(int index)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.getAt(index);
}

int RoomService::getRoomCount()
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.count();
}



