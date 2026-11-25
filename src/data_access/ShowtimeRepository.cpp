#include "ShowtimeRepository.h"
#include "../models/Movie.h"
#include "../models/Room.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;


// Kiem tra toan bo du lieu Showtime
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

// Lay ID Showtime
string ShowtimeRepository::getShowtimeId(Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return "";
    }
    return showtime->getShowtimeId();
}

// Chuyen mot dong TXT thanh Showtime
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

// Chuyen Showtime thanh mot dong TXT
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

ShowtimeRepository::ShowtimeRepository(const string& filePath, IRepository<Movie>& movieRepo, IRepository<Room>& roomRepo)
    : repo(filePath, getShowtimeId, stringToShowtime, showtimeToString),
      movieRepo(movieRepo), roomRepo(roomRepo)
{
}

ShowtimeRepository::~ShowtimeRepository()
{
    try
    {
        unlinkAll();
    }
    catch (const exception& e)
    {
        cerr << "Loi huy ShowtimeRepository: " << e.what() << endl;
        terminate();
    }
    catch (...)
    {
        cerr << "Loi huy ShowtimeRepository khong xac dinh" << endl;
        terminate();
    }
}

// Kiem tra tham chieu Movie va Room
bool ShowtimeRepository::isValidReference(const Showtime* showtime)
{
    if (showtime == nullptr)
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
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
        (!movie->isActive() || !room->isActive()))
    {
        return false;
    }
    return true;
}

// Kiem tra trung lich trong cung phong
bool ShowtimeRepository::hasScheduleConflict(const Showtime* showtime, const string& excludeId)
{
    if (showtime == nullptr || showtime->getStatus() == ShowtimeStatus::CANCELLED)
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* current = repo.getAt(i);
        if (current == nullptr || current->getShowtimeId() == excludeId ||
            current->getRoomId() != showtime->getRoomId() ||
            current->getStatus() == ShowtimeStatus::CANCELLED)
        {
            continue;
        }
        if (DateTime::isOverlapping(
                current->getStartAt(), current->getEndAt(),
                showtime->getStartAt(), showtime->getEndAt()))
        {
            return true;
        }
    }
    return false;
}

// Go lien ket Movie va Room cua mot Showtime
void ShowtimeRepository::unlinkReferences(Showtime* showtime)
{
    Movie* movie = showtime->getMovie();
    Room* room = showtime->getRoom();
    bool movieUnlinked = true;
    bool roomUnlinked = true;
    if (movie != nullptr)
    {
        movieUnlinked = movie->unlinkShowtime(showtime);
    }
    if (room != nullptr)
    {
        roomUnlinked = room->unlinkShowtime(showtime);
    }
    if (!movieUnlinked || !roomUnlinked)
    {
        throw runtime_error("Khong the go lien ket Movie/Room cua Showtime");
    }
}

// Go lien ket cua cac Showtime dang duoc quan ly
void ShowtimeRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        unlinkReferences(repo.getAt(i));
    }
}

// Khoi phuc Showtime sau khi cap nhat that bai
void ShowtimeRepository::restoreShowtime(Showtime* showtime,
                                         const Showtime& oldValues,
                                         Movie* oldMovie,
                                         Room* oldRoom)
{
    // Khoi phuc lien ket Movie
    if (showtime->getMovie() != oldMovie)
    {
        Movie* movie = showtime->getMovie();

        if (movie != nullptr && !movie->unlinkShowtime(showtime))
        {
            throw runtime_error("Khong the huy lien ket Movie moi");
        }

        if (!showtime->setMovieId(oldValues.getMovieId()) ||
            !showtime->linkMovie(oldMovie))
        {
            throw runtime_error("Khong the khoi phuc lien ket Movie");
        }
    }

    // Khoi phuc lien ket Room
    if (showtime->getRoom() != oldRoom)
    {
        Room* room = showtime->getRoom();

        if (room != nullptr && !room->unlinkShowtime(showtime))
        {
            throw runtime_error("Khong the huy lien ket Room moi");
        }

        if (!showtime->setRoomId(oldValues.getRoomId()) ||
            !showtime->linkRoom(oldRoom))
        {
            throw runtime_error("Khong the khoi phuc lien ket Room");
        }
    }

    // Khoi phuc cac thuoc tinh cu
    showtime->setStartAt(oldValues.getStartAt());
    showtime->setEndAt(oldValues.getEndAt());
    showtime->setStatus(oldValues.getStatus());
}

// Doc du lieu Showtime tu file
bool ShowtimeRepository::load()
{
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
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
            Showtime* showtime = repo.getAt(i);
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());
            if (movie == nullptr || room == nullptr)
            {
                throw runtime_error("Showtime tham chieu Movie hoac Room khong ton tai");
            }
            if (hasScheduleConflict(showtime, showtime->getShowtimeId()))
            {
                throw runtime_error("Trung lich Showtime trong file");
            }
            if (!showtime->linkMovie(movie) || !showtime->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Showtime voi Movie va Room");
            }
        }
    }
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}

// Luu du lieu Showtime xuong file
bool ShowtimeRepository::save()
{
    return repo.save();
}

// Them suat chieu moi
bool ShowtimeRepository::add(Showtime* showtime)
{
    if (!repo.isLoaded() || !isValidShowtime(showtime))
    {
        return false;
    }
    // Khong them Showtime da duoc lien ket
    if (showtime->getMovie() != nullptr || showtime->getRoom() != nullptr)
    {
        return false;
    }
    // Can kiem tra truoc khi tao lien ket
    if (repo.findById(showtime->getShowtimeId()) != nullptr)
    {
        return false;
    }
    if (!isValidReference(showtime) || hasScheduleConflict(showtime))
    {
        return false;
    }

    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());

    bool added = false;
    try
    {
        if (showtime->linkMovie(movie) && showtime->linkRoom(room))
        {
            added = repo.add(showtime);
        }
    }
    catch (...)
    {
        unlinkReferences(showtime);
        throw;
    }
    if (!added)
    {
        unlinkReferences(showtime);
        return false;
    }
    return true;
}

// Cap nhat suat chieu
bool ShowtimeRepository::update(const Showtime& newValues)
{
    if (!repo.isLoaded() || !isValidShowtime(&newValues))
    {
        return false;
    }
    Showtime* current = repo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
    }
    if (!isValidReference(&newValues) ||
        hasScheduleConflict(&newValues, current->getShowtimeId()))
    {
        return false;
    }

    Movie* oldMovie = current->getMovie();
    Room* oldRoom = current->getRoom();
    
    Movie* newMovie = movieRepo.findById(newValues.getMovieId());
    Room* newRoom = roomRepo.findById(newValues.getRoomId());

    if (oldMovie == nullptr || oldRoom == nullptr ||
        newMovie == nullptr || newRoom == nullptr)
    {
        return false;
    }
    const bool movieChanged = oldMovie != newMovie;
    const bool roomChanged = oldRoom != newRoom;

    Showtime oldValues(current->getShowtimeId(),
                       current->getMovieId(),
                       current->getRoomId(),
                       current->getStartAt(),
                       current->getEndAt(),
                       current->getStatus());
    try
    {
        if (movieChanged)
        {
            if (!oldMovie->unlinkShowtime(current) ||
                !current->setMovieId(newValues.getMovieId()) ||
                !current->linkMovie(newMovie))
            {
                throw runtime_error("Khong the doi Movie cua Showtime");
            }
        }
        if (roomChanged)
        {
            if (!oldRoom->unlinkShowtime(current) ||
                !current->setRoomId(newValues.getRoomId()) ||
                !current->linkRoom(newRoom))
            {
                throw runtime_error("Khong the doi Room cua Showtime");
            }
        }
        current->setStartAt(newValues.getStartAt());
        current->setEndAt(newValues.getEndAt());
        current->setStatus(newValues.getStatus());
    }
    catch (...)
    {
        restoreShowtime(current, oldValues, oldMovie, oldRoom);
        throw;
    }
    return true;
}

// Xoa suat chieu theo ID
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
    if (!room->unlinkShowtime(showtime))
    {
        if (!movie->linkShowtime(showtime))
        {
            throw runtime_error("Khong the khoi phuc lien ket Movie");
        }
        return false;
    }
    if (!repo.removeById(showtimeId))
    {
        bool movieRestored = movie->linkShowtime(showtime);
        bool roomRestored = room->linkShowtime(showtime);
        if (!movieRestored || !roomRestored)
        {
            throw runtime_error("Khong the khoi phuc lien ket Showtime");
        }
        return false;
    }
    return true;
}

// Tim suat chieu theo ID
Showtime* ShowtimeRepository::findById(const string& showtimeId)
{
    return repo.findById(showtimeId);
}

// Lay suat chieu theo vi tri
Showtime* ShowtimeRepository::getAt(int index)
{
    return repo.getAt(index);
}

// Dem so luong suat chieu
int ShowtimeRepository::count()
{
    return repo.count();
}

// Kiem tra trang thai load
bool ShowtimeRepository::isLoaded()
{
    return repo.isLoaded();
}
