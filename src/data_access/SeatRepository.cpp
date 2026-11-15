#include "SeatRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>

using namespace std;

SeatRepository::SeatRepository(const string& filePath, RoomRepository& roomRepo)
    : repo(filePath, getSeatKey, stringToSeat, seatToString), roomRepo(roomRepo)
{}

string SeatRepository::getSeatKey(Seat* seat) //(roomId + seatId)
{
    if (seat == nullptr)
    {
        return "";
    }
    return seat->getRoomId() + "|" + seat->getSeatId();
}

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
    if (seat->getSeatId().empty() || seat->getSeatId().find('|') != string::npos)
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

bool SeatRepository::save()
{
    return repo.save();
}

// load dữ liệu từ txt tạo thành đối tượng ghế rồi liên kết phòng với ghế
bool SeatRepository::load()
{
    if (repo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Seat* seat = repo.getAt(i); //tìm ghế trong danh sách ghế
        Room* room = roomRepo.findById(seat->getRoomId()); //tìm phòng tương ứng từ ghế đó

        if (room == nullptr || !seat->linkRoom(room)) //nếu k tìm thấy phòng hoặc có phòng nhưng ko link được ghế
        {
            // xóa liên kết
            for (int j = 0; j < i; j++) //j là duyệt lại những ghế trước đó đã liên kết (nếu dính 1 ghế lỗi(ghế thứ i))
            {
                Seat* linkedSeat = repo.getAt(j);
                if (linkedSeat->getRoom() != nullptr)
                {
                    linkedSeat->getRoom()->unlinkSeat(linkedSeat);
                }
            }
            repo.clear(); //xóa đối tượng seat
            return false;
        }
    }
    return true;
}

bool SeatRepository::add(Seat* seat)
{
    if (!repo.isLoaded() || !roomRepo.isLoaded() || !isValidSeat(seat))
    {
        return false;
    }
    Room* room = roomRepo.findById(seat->getRoomId());
    if (room == nullptr)
    {
        return false;
    }
    // duyệt các ghế có trong phòng xem thử có trùng không, ko cho thêm trùng
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == seat)
        {
            return false;
        }
    }
    // kiểm tra ghế đã liên kết chưa, không thêm ghế đã liên kết
    if (seat->getRoom() != nullptr)
    {
        return false;
    }
    // kiểm tra và liên kết ghế với phòng
    if (!seat->linkRoom(room))
    {
        return false;
    }
    // Thêm ghế vào danh sách
    if (!repo.add(seat))
    {
        room->unlinkSeat(seat);
        return false;
    }
    return true;
}

bool SeatRepository::update(const Seat& newValues)
{
    if (!repo.isLoaded() || !isValidSeat(&newValues))
    {
        return false;
    }
    Seat* seat = findById(newValues.getRoomId(), newValues.getSeatId());
    if (seat == nullptr)
    {
        return false;
    }
    // không được thay đổi vị trí ghế
    if (seat->getRow() != newValues.getRow() || seat->getColumn() != newValues.getColumn())
    {
        return false;
    }
    seat->setSeatType(newValues.getSeatType());
    return true;
}

bool SeatRepository::removeById(const string& roomId, const string& seatId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Seat* seat = findById(roomId, seatId);
    if (seat == nullptr)
    {
        return false;
    }
    Room* room = seat->getRoom();
    if (room == nullptr)
    {
        return false;
    }
    // Thao lien ket truoc khi xoa Seat
    if (!room->unlinkSeat(seat))
    {
        return false;
    }
    // Xoa Seat khoi Repository va giai phong bo nho
    return repo.removeById(roomId + "|" + seatId);
}

Seat* SeatRepository::findById(const string& roomId, const string& seatId)
{
    return repo.findById(roomId + "|" + seatId);
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


