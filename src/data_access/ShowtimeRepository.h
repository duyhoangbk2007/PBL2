#ifndef SHOWTIME_REPOSITORY_H
#define SHOWTIME_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Showtime.h"

class Movie;
class Room;

class ShowtimeRepository : public IRepository<Showtime>
{
private:
    Repository<Showtime> repo;
    IRepository<Movie>& movieRepo;
    IRepository<Room>& roomRepo;

    static std::string getShowtimeId(Showtime* showtime);
    static Showtime* stringToShowtime(const std::string& line);
    static std::string showtimeToString(Showtime* showtime);
    static bool isValidShowtime(const Showtime* showtime);

    bool isValidReference(const Showtime* showtime);
    bool hasScheduleConflict(const Showtime* showtime, const std::string& excludeId = "");
    void unlinkReferences(Showtime* showtime);
    void unlinkAll();
    void restoreShowtime(Showtime* showtime, const Showtime& oldValues, Movie* oldMovie, Room* oldRoom);

public:
    ShowtimeRepository(const std::string& filePath, IRepository<Movie>& movieRepo, IRepository<Room>& roomRepo);
    ~ShowtimeRepository();

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Showtime* showtime) override;
    bool update(const Showtime& newValues) override;
    bool removeById(const std::string& showtimeId) override;

    // Truy van du lieu
    Showtime* findById(const std::string& showtimeId) override;
    Showtime* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
