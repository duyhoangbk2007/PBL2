#ifndef SHOWTIME_REPOSITORY_H
#define SHOWTIME_REPOSITORY_H

#include <string>
#include "Repository.h"
#include "../models/Showtime.h"

class MovieRepository;
class RoomRepository;

class ShowtimeRepository
{
private:
    Repository<Showtime> repo;
    MovieRepository& movieRepo;
    RoomRepository& roomRepo;

    static std::string getShowtimeId(Showtime* showtime);
    static Showtime* stringToShowtime(const std::string& line);
    static std::string showtimeToString(Showtime* showtime);
    static bool isValidShowtime(const Showtime* showtime);

    // Kiem tra quan he va trung lich
    bool isValidReference(const Showtime* showtime);
    bool hasScheduleConflict(const Showtime* showtime, const std::string& excludeId = "");

public:
    ShowtimeRepository(const std::string& filePath, MovieRepository& movieRepo, RoomRepository& roomRepo);
    ~ShowtimeRepository();
    bool load();
    bool save();

    bool add(Showtime* showtime);
    bool update(const Showtime& newValues);
    bool removeById(const std::string& showtimeId);

    // Truy van suat chieu
    Showtime* findById(const std::string& showtimeId);
    Showtime* getAt(int index);
    int count();
    bool isLoaded();
};

#endif
