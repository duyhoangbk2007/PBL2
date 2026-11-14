#ifndef ROOM_SERVICE_H
#define ROOM_SERVICE_H

#include <string>
#include "../data_access/RoomRepository.h"
#include "AuthService.h"

class RoomService
{
private:
    RoomRepository& roomRepo;
    AuthService& authService;

public:
    RoomService(RoomRepository& roomRepo, AuthService& authService);

    bool load();
    bool save();

    bool addRoom(Room* room);
    bool updateRoom(const Room& newValues);
    bool removeRoom(const std::string& roomId);

    const Room* findRoomById(const std::string& roomId);
    const Room* getRoomAt(int index);
    int getRoomCount();
};

#endif