#ifndef SHOWTIME_SERVICE_H
#define SHOWTIME_SERVICE_H

#include <string>
#include "../data_access/ShowtimeRepository.h"
#include "../data_access/MovieRepository.h"
#include "../data_access/RoomRepository.h"
#include "AuthService.h"

class ShowtimeService
{
private:
    ShowtimeRepository& showtimeRepo;
    MovieRepository& movieRepo;
    RoomRepository& roomRepo;
    AuthService& authService;

    bool isValidSchedule(const Showtime& showtime);

public:
    ShowtimeService(ShowtimeRepository& showtimeRepo, MovieRepository& movieRepo, RoomRepository& roomRepo, AuthService& authService);

    bool load();
    bool save();

    bool addShowtime(Showtime* showtime);
    bool updateShowtime(const Showtime& newValues);
    bool cancelShowtime(const std::string& showtimeId);
    bool removeShowtime(const std::string& showtimeId);

    const Showtime* findShowtimeById(const std::string& showtimeId);
    const Showtime* getShowtimeAt(int index);
    int getShowtimeCount();
};

#endif
