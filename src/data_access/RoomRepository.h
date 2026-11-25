#ifndef ROOM_REPOSITORY_H
#define ROOM_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Room.h"

using namespace std;

class RoomRepository : public IRepository<Room>
{
private:
    Repository<Room> repo;

    static string getRoomId(Room* room);
    static Room* stringToRoom(const string& line);
    static string roomToString(Room* room);

    static bool isValidRoom(const Room* room);

public:
    RoomRepository(const string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Room* room) override;
    bool update(const Room& newValues) override;
    bool removeById(const string& roomId) override;

    // Truy van du lieu
    Room* findById(const string& roomId) override;
    Room* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
