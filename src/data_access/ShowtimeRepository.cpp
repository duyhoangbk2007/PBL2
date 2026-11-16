#include "ShowtimeRepository.h"
#include "MovieRepository.h"
#include "RoomRepository.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <stdexcept>
#include <vector>

using namespace std;

ShowtimeRepository::ShowtimeRepository(const string& filePath, MovieRepository& movieRepo, RoomRepository& roomRepo)
    : repo(filePath, getShowtimeId, stringToShowtime, showtimeToString), movieRepo(movieRepo), roomRepo(roomRepo)
{
}
ShowtimeRepository::~ShowtimeRepository()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* showtime = repo.getAt(i);
        if (showtime->getMovie() != nullptr)
        {
            showtime->getMovie()->unlinkShowtime(showtime);
        }
        if (showtime->getRoom() != nullptr)
        {
            showtime->getRoom()->unlinkShowtime(showtime);
        }
    }
}

string ShowtimeRepository::getShowtimeId(Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return "";
    }
    return showtime->getShowtimeId();
}

bool ShowtimeRepository::isValidShowtime(const Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(showtime->getShowtimeId(), "SHW"))
    {
        return false;
    }
    if (!Validator::isValidId(showtime->getMovieId(), "MOV") ||
        !Validator::isValidId(showtime->getRoomId(), "ROM"))
    {
        return false;
    }
    if (!Validator::isValidDateTime(showtime->getStartAt()) ||
        !Validator::isValidDateTime(showtime->getEndAt()))
    {
        return false;
    }
    if (!DateTime::isBefore(showtime->getStartAt(), showtime->getEndAt()))
    {
        return false;
    }
    if (showtime->getStatus() != ShowtimeStatus::SCHEDULED &&
        showtime->getStatus() != ShowtimeStatus::CANCELLED)
    {
        return false;
    }
    return true;
}

// kiểm tra phim và phòng của suất chiếu
bool ShowtimeRepository::isValidReference(const Showtime* showtime)
{
    if (!isValidShowtime(showtime))
    {
        return false;
    }
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());
    if (movie == nullptr || room == nullptr)
    {
        return false;
    }
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED)
    {
        if (!movie->isActive() || !room->isActive())
        {
            return false;
        }
    }
    return true;
}

bool ShowtimeRepository::hasScheduleConflict(const Showtime* showtime, const string& excludeId)
{ 
    if (showtime == nullptr)
    {
        return false;
    }
    if (showtime->getStatus() == ShowtimeStatus::CANCELLED)
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* current = repo.getAt(i);
        if (current == nullptr)
        {
            continue;
        }
        // Bo qua chinh suat dang cap nhat
        if (current->getShowtimeId() == excludeId) // excludeId là Id của suất chiếu đang cập nhật, cần bỏ qua khi update suất chiếu
        {
            continue;
        }
        // Chi kiem tra cac suat cung phong
        if (current->getRoomId() != showtime->getRoomId())
        {
            continue;
        }
        // Bo qua suat da huy
        if (current->getStatus() == ShowtimeStatus::CANCELLED)
        {
            continue;
        }

        if (DateTime::isOverlapping(
                current->getStartAt(),
                current->getEndAt(),
                showtime->getStartAt(),
                showtime->getEndAt()))
        {
            return true;
        }
    }
    return false;
}

Showtime* ShowtimeRepository::stringToShowtime(const string& line)
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
    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Showtime");
    }
    ShowtimeStatus status;
    if (fields[5] == "SCHEDULED")
    {
        status = ShowtimeStatus::SCHEDULED;
    }
    else if (fields[5] == "CANCELLED")
    {
        status = ShowtimeStatus::CANCELLED;
    }
    else
    {
        throw runtime_error("Trang thai Showtime khong hop le");
    }
    Showtime* showtime = new Showtime(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        fields[4],
        status
    );

    if (!isValidShowtime(showtime))
    {
        delete showtime;
        throw runtime_error("Du lieu Showtime khong hop le");
    }
    return showtime;
}

string ShowtimeRepository::showtimeToString(Showtime* showtime)
{
    if (!isValidShowtime(showtime))
    {
        throw runtime_error("Showtime khong hop le khi ghi file");
    }
    string status;
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED)
    {
        status = "SCHEDULED";
    }
    else
    {
        status = "CANCELLED";
    }

    return showtime->getShowtimeId() + "|" +
           showtime->getMovieId() + "|" +
           showtime->getRoomId() + "|" +
           showtime->getStartAt() + "|" +
           showtime->getEndAt() + "|" +
           status;
}

bool ShowtimeRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
    }
    // 1. Doc du lieu Showtime tu file
    if (!repo.load())
    {
        return false;
    }
    // 2. Kiem tra tham chieu va trung lich
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());

            if (movie == nullptr || room == nullptr)
            {
                throw runtime_error("Showtime tham chieu Movie hoac Room khong ton tai");
            }
            // Kiem tra trung lich voi cac suat truoc do
            for (int j = 0; j < i; j++)
            {
                Showtime* other = repo.getAt(j);
                if (showtime->getStatus() == ShowtimeStatus::CANCELLED ||
                    other->getStatus() == ShowtimeStatus::CANCELLED)
                {
                    continue;
                }
                if (showtime->getRoomId() == other->getRoomId() &&
                    DateTime::isOverlapping(
                        showtime->getStartAt(),
                        showtime->getEndAt(),
                        other->getStartAt(),
                        other->getEndAt()))
                {
                    throw runtime_error("Trung lich Showtime trong file");
                }
            }
        }

        // Lien ket Showtime voi Movie va Room
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());
            if (!showtime->linkMovie(movie) || !showtime->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Showtime voi Movie va Room");
            }
        }
    }
    catch (...)
    {
        // Hoan tac cac lien ket da tao
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
            if (showtime->getMovie() != nullptr)
            {
                showtime->getMovie()->unlinkShowtime(showtime);
            }
            if (showtime->getRoom() != nullptr)
            {
                showtime->getRoom()->unlinkShowtime(showtime);
            }
        }

        repo.clear();
        throw;
    }

    return true;
}

bool ShowtimeRepository::save()
{
    return repo.save();
}

Showtime* ShowtimeRepository::findById(const string& showtimeId)
{
    return repo.findById(showtimeId);
}

Showtime* ShowtimeRepository::getAt(int index)
{
    return repo.getAt(index);
}

int ShowtimeRepository::count()
{
    return repo.count();
}

bool ShowtimeRepository::isLoaded()
{
    return repo.isLoaded();
}

bool ShowtimeRepository::add(Showtime* showtime)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    if (!isValidShowtime(showtime))
    {
        return false;
    }
    // không thêm Showtime đã được liên kết từ trước
    if (showtime->getMovie() != nullptr || showtime->getRoom() != nullptr)
    {
        return false;
    }
    // kiểm tra ID trùng
    if (repo.findById(showtime->getShowtimeId()) != nullptr)
    {
        return false;
    }
    // 4. Kiem tra Movie va Room
    if (!isValidReference(showtime))
    {
        return false;
    }
    // 5. Kiem tra trung lich
    if (hasScheduleConflict(showtime))
    {
        return false;
    }
    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());
    if (!showtime->linkMovie(movie))
    {
        return false;
    }
    if (!showtime->linkRoom(room))
    {
        movie->unlinkShowtime(showtime);
        return false;
    }
    //nếu liên kết thành công mà ko thêm vào Repo, Nếu không tháo liên kết, Movie và Room sẽ giữ một Showtime không thuộc danh sách quản lý của Repository.
    if (!repo.add(showtime))
    {
        room->unlinkShowtime(showtime);
        movie->unlinkShowtime(showtime);
        return false;
    }
    return true;
}

bool ShowtimeRepository::update(const Showtime& newValues)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Showtime* current = repo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
    }
    // 3. Khong cho doi Movie va Room
    if (current->getMovieId() != newValues.getMovieId() ||
        current->getRoomId() != newValues.getRoomId())
    {
        return false;
    }
    // 4. Kiem tra du lieu moi
    if (!isValidShowtime(&newValues))
    {
        return false;
    }
    if (!isValidReference(&newValues))
    {
        return false;
    }
    // 5. Kiem tra trung lich
    if (hasScheduleConflict(&newValues, current->getShowtimeId()))
    {
        return false;
    }
    current->setStartAt(newValues.getStartAt());
    current->setEndAt(newValues.getEndAt());
    current->setStatus(newValues.getStatus());
    return true;
}

bool ShowtimeRepository::removeById(const string& showtimeId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Showtime* showtime = repo.findById(showtimeId);
    if (showtime == nullptr)
    {
        return false;
    }
    Movie* movie = showtime->getMovie();
    Room* room = showtime->getRoom();
    if (movie == nullptr || room == nullptr)
    {
        return false;
    }
    if (!movie->unlinkShowtime(showtime))
    {
        return false;
    }
    //nếu k hủy liên kết showtime với room được thì phải khôi phục lại liên kết showtime với movie
    if (!room->unlinkShowtime(showtime))
    {
        if (!movie->linkShowtime(showtime))
        {
            throw runtime_error(
                "Khong the khoi phuc lien ket Movie"
            );
        }
        return false;
    }
    return repo.removeById(showtimeId);
}

