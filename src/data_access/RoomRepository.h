#ifndef ROOM_REPOSITORY_H
#define ROOM_REPOSITORY_H

#include <string>
#include "Repository.h"
#include "../models/Room.h"

class RoomRepository
{
private:
    Repository<Room> repo;

    static std::string getRoomId(Room* room);
    static Room* parseRoom(const std::string& line);
    static std::string serializeRoom(Room* room);
    static bool isValidRoom(const Room* room);

public:
    RoomRepository(const std::string& filePath);

    bool load();
    bool save();

    bool add(Room* room);
    bool update(const Room& newValues);
    bool removeById(const std::string& roomId);

    Room* findById(const std::string& roomId);
    Room* getAt(int index);

    int count();
    bool isLoaded();
};

#endif
