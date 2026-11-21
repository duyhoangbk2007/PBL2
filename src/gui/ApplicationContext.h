#pragma once

#include "../business/AuthService.h"
#include "../business/MovieService.h"
#include "../business/RoomService.h"
#include "../business/SeatService.h"
#include "../data_access/ShowtimeRepository.h"
#include <QString>

class ApplicationContext
{
private:
    UserRepository users;
    RolePermissionRepository permissions;

    MovieRepository movieRepo;
    RoomRepository roomRepo;
    SeatRepository seatRepo;
    ShowtimeRepository showtimeRepo;

public:
    explicit ApplicationContext(const QString& dataDirectory);
    QString load();
    AuthService auth;
    MovieService movies;
    RoomService rooms;
    SeatService seats;
    QString catalogError;
};
