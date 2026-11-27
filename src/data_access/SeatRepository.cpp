#include "SeatRepository.h"
#include "../models/Room.h"
#include "../utils/Validator.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

bool SeatRepository::isValidSeat(const Seat* seat)
{
    if (seat == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(seat->getRoomId(), "ROM"))
    {
        return false;
    }
    if (!Validator::isValidText(seat->getSeatId()))
    {
        return false;
    }
    if (seat->getRow() <= 0 || seat->getColumn() <= 0)
    {
        return false;
    }

    SeatType type = seat->getSeatType();
    if (type != SeatType::STANDARD &&
        type != SeatType::VIP &&
        type != SeatType::COUPLE)
    {
        return false;
    }
    return true;
}

string SeatRepository::getSeatKey(Seat* seat)
{
    if (seat == nullptr)
    {
        return "";
    }
    // Khoa ghe gom ma phong va ma ghe.
    return seat->getRoomId() + "|" + seat->getSeatId();
}

Seat* SeatRepository::stringToSeat(const string& line)
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
        throw runtime_error("Sai so luong truong Seat");
    }

    int row;
    int column;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        row = stoi(fields[2], &pos1);
        column = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() ||
            pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Hang hoac cot Seat khong hop le");
    }

    SeatType seatType;
    if (fields[4] == "STANDARD")
    {
        seatType = SeatType::STANDARD;
    }
    else if (fields[4] == "VIP")
    {
        seatType = SeatType::VIP;
    }
    else if (fields[4] == "COUPLE")
    {
        seatType = SeatType::COUPLE;
    }
    else
    {
        throw runtime_error("Loai ghe khong hop le");
    }

    Seat* seat = new Seat(
        fields[0],
        fields[1],
        row,
        column,
        seatType
    );
    if (!isValidSeat(seat))
    {
        delete seat;
        throw runtime_error("Du lieu Seat khong hop le");
    }
    return seat;
}

string SeatRepository::seatToString(Seat* seat)
{
    if (!isValidSeat(seat))
    {
        throw runtime_error("Seat khong hop le khi ghi file");
    }

    string type;
    switch (seat->getSeatType())
    {
        case SeatType::STANDARD:
            type = "STANDARD";
            break;

        case SeatType::VIP:
            type = "VIP";
            break;

        case SeatType::COUPLE:
            type = "COUPLE";
            break;

        default:
            throw runtime_error("Loai ghe khong hop le");
    }

    return seat->getRoomId() + "|" +
           seat->getSeatId() + "|" +
           to_string(seat->getRow()) + "|" +
           to_string(seat->getColumn()) + "|" +
           type;
}

SeatRepository::SeatRepository(const string& filePath,
                               IRepository<Room>& roomRepo)
    : repo(filePath, getSeatKey, stringToSeat, seatToString),
      roomRepo(roomRepo)
{
}

SeatRepository::~SeatRepository()
{
    // Go lien ket truoc khi repo huy cac Seat dang so huu.
    try
    {
        unlinkAll();
    }
    catch (const exception& e)
    {
        cerr << "Loi huy SeatRepository: " << e.what() << endl;
        terminate();
    }
    catch (...)
    {
        cerr << "Loi huy SeatRepository khong xac dinh" << endl;
        terminate();
    }
}

void SeatRepository::unlinkReferences(Seat* seat)
{
    if (seat == nullptr)
    {
        return;
    }

    Room* room = seat->getRoom();
    if (room != nullptr && !room->unlinkSeat(seat))
    {
        throw runtime_error("Khong the thao lien ket Seat va Room");
    }
}

void SeatRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        unlinkReferences(repo.getAt(i));
    }
}

bool SeatRepository::load()
{
    if (!roomRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Seat* seat = repo.getAt(i);
            Room* room = roomRepo.findById(seat->getRoomId());

            if (room == nullptr)
            {
                throw runtime_error("Seat tham chieu Room khong ton tai");
            }
            if (!seat->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Seat voi Room");
            }
        }
    }
    catch (...)
    {
        // Go lien ket truoc khi xoa Seat da nap.
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}

bool SeatRepository::save()
{
    return repo.save();
}

bool SeatRepository::add(Seat* seat)
{
    if (!repo.isLoaded() || !roomRepo.isLoaded() ||
        !isValidSeat(seat))
    {
        return false;
    }
    // Tu choi ghe da lien ket hoac trung khoa truoc khi thay doi Room.
    if (seat->getRoom() != nullptr)
    {
        return false;
    }
    if (findById(getSeatKey(seat)) != nullptr)
    {
        return false;
    }

    Room* room = roomRepo.findById(seat->getRoomId());
    if (room == nullptr)
    {
        return false;
    }

    // Lien ket Room truoc; repo chi nhan quyen so huu khi them thanh cong.
    try
    {
        if (seat->linkRoom(room) && repo.add(seat))
        {
            return true;
        }
    }
    catch (...)
    {
        // Thu hoi lien ket neu thao tac them nem loi.
        unlinkReferences(seat);
        throw;
    }

    // Caller van so huu Seat neu them that bai.
    unlinkReferences(seat);
    return false;
}

bool SeatRepository::update(const Seat& newValues)
{
    if (!repo.isLoaded() || !isValidSeat(&newValues))
    {
        return false;
    }

    Seat* seat = findById(newValues.getRoomId(),
                          newValues.getSeatId());

    if (seat == nullptr)
    {
        return false;
    }
    // Vi tri ghe bat bien; chi cho phep doi loai ghe.
    if (seat->getRow() != newValues.getRow() ||
        seat->getColumn() != newValues.getColumn())
    {
        return false;
    }

    seat->setSeatType(newValues.getSeatType());
    return true;
}

bool SeatRepository::removeById(const string& key)
{
    if (!repo.isLoaded())
    {
        return false;
    }

    Seat* seat = repo.findById(key);
    if (seat == nullptr)
    {
        return false;
    }

    Room* room = seat->getRoom();
    if (room == nullptr)
    {
        return false;
    }

    // Go khoi Room truoc khi repo giai phong Seat.
    if (!room->unlinkSeat(seat))
    {
        return false;
    }

    if (!repo.removeById(key))
    {
        // Khoi phuc lien ket neu Seat van con trong repo.
        if (!room->linkSeat(seat))
        {
            throw runtime_error("Khong the khoi phuc lien ket Seat");
        }
        return false;
    }
    return true;
}

bool SeatRepository::removeById(const string& roomId,
                                const string& seatId)
{
    return removeById(roomId + "|" + seatId);
}

Seat* SeatRepository::findById(const string& key)
{
    return repo.findById(key);
}

Seat* SeatRepository::findById(const string& roomId,
                               const string& seatId)
{
    return findById(roomId + "|" + seatId);
}

Seat* SeatRepository::getAt(int index)
{
    return repo.getAt(index);
}

int SeatRepository::count()
{
    return repo.count();
}

bool SeatRepository::isLoaded()
{
    return repo.isLoaded();
}
