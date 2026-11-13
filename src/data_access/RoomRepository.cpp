#include "RoomRepository.h"
#include "../utils/Validator.h"
#include "../models/Seat.h"
#include <stdexcept>
#include <vector>

using namespace std;

RoomRepository::RoomRepository(const string& filePath)
    : repo(filePath, getRoomId, parseRoom, serializeRoom)
{}

string RoomRepository::getRoomId(Room* room)
{
    if (room == nullptr)
    {
        return "";
    }
    return room->getRoomId();
}

bool RoomRepository::isValidRoom(const Room* room)
{
    if (room == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(room->getRoomId(), "ROM"))
    {
        return false;
    }
    if (room->getName().empty())
    {
        return false;
    }
    if (room->getName().find('|') != string::npos)
    {
        return false;
    }
    if (room->getRowCount() <= 0)
    {
        return false;
    }
    if (room->getColumnCount() <= 0)
    {
        return false;
    }
    return true;
}
 
Room* RoomRepository::parseRoom(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    if (fields.size() != 5)
    {
        throw runtime_error("Sai so luong truong Room");
    }
    int rowCount;
    int columnCount;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        rowCount = stoi(fields[2], &pos1);
        columnCount = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() || pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("So hang hoac so cot khong hop le");
    }
    // Kiem tra trang thai
    if (fields[4] != "0" && fields[4] != "1")
    {
        throw runtime_error("Trang thai Room khong hop le");
    }
    bool active = (fields[4] == "1");

    // Tao doi tuong Room
    Room* room = new Room(
        fields[0],
        fields[1],
        rowCount,
        columnCount,
        active
    );

    // Kiem tra du lieu Room
    if (!isValidRoom(room))
    {
        delete room;
        throw runtime_error("Du lieu Room khong hop le");
    }
    return room;
}

string RoomRepository::serializeRoom(Room* room)
{
    if (!isValidRoom(room))
    {
        throw runtime_error("Room khong hop le khi ghi file");
    }
    return room->getRoomId() + "|" +
           room->getName() + "|" +
           to_string(room->getRowCount()) + "|" +
           to_string(room->getColumnCount()) + "|" +
           (room->isActive() ? "1" : "0");
}

bool RoomRepository::save()
{
    return repo.save();
}

bool RoomRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    return repo.load();
}

bool RoomRepository::add(Room* room)
{
    if (!repo.isLoaded() || !isValidRoom(room))
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == room)
        {
            return false;
        }
    }
    return repo.add(room);
}

bool RoomRepository::update(const Room& newValues)
{
    if (!repo.isLoaded() || !isValidRoom(&newValues))
    {
        return false;
    }
    Room* room = repo.findById(newValues.getRoomId());

    if (room == nullptr)
    {
        return false;
    }
    // Khong cho thu nho phong neu co ghe nam ngoai kich thuoc moi
    for (int i = 0; i < room->getSeatCount(); i++)
    {
        Seat* seat = room->getSeatAt(i);
        if (seat == nullptr)
        {
            return false;
        }
        if (seat->getRow() > newValues.getRowCount() || seat->getColumn() > newValues.getColumnCount())
        {
            return false;
        }
    }
    room->setName(newValues.getName());
    room->setRowCount(newValues.getRowCount());
    room->setColumnCount(newValues.getColumnCount());
    room->setActive(newValues.isActive());
    return true;
}

bool RoomRepository::removeById(const string& roomId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Room* room = repo.findById(roomId);

    if (room == nullptr)
    {
        return false;
    }
    // Khong xoa phong khi con ghe hoac suat chieu
    if (room->getSeatCount() > 0 ||
        room->getShowtimeCount() > 0)
    {
        return false;
    }
    return repo.removeById(roomId);
}

Room* RoomRepository::findById(const string& roomId)
{
    return repo.findById(roomId);
}
Room* RoomRepository::getAt(int index)
{
    return repo.getAt(index);
}
int RoomRepository::count()
{
    return repo.count();
}
bool RoomRepository::isLoaded()
{
    return repo.isLoaded();
}



