#ifndef ROOM_SERVICE_H
#define ROOM_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/Room.h"
#include "AuthService.h"

class RoomService
{
private:
    IRepository<Room>& roomRepo;
    AuthService& authService;

public:
    RoomService(IRepository<Room>& roomRepo,
                AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly phong
    bool addRoom(Room* room);
    bool updateRoom(const Room& newValues);
    bool removeRoom(const std::string& roomId);
    bool setRoomActive(const std::string& roomId, bool active);

    // Tra cuu phong
    const Room* findRoomById(const std::string& roomId);
    const Room* getRoomAt(int index);
    int getRoomCount();
};

#endif
