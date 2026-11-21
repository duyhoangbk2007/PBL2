#include "ApplicationContext.h"

#include <QDir>
#include <exception>

ApplicationContext::ApplicationContext(const QString& dataDirectory)
    : users(QDir(dataDirectory).filePath("users.txt").toStdString()),
      permissions(QDir(dataDirectory).filePath("role_permissions.txt").toStdString()),
      movieRepo(QDir(dataDirectory).filePath("movies.txt").toStdString()),
      roomRepo(QDir(dataDirectory).filePath("rooms.txt").toStdString()),
      seatRepo(QDir(dataDirectory).filePath("seats.txt").toStdString(), roomRepo),
      showtimeRepo(QDir(dataDirectory).filePath("showtimes.txt").toStdString(), movieRepo, roomRepo),
      auth(users, permissions), movies(movieRepo, auth), rooms(roomRepo, auth), seats(seatRepo, auth)
{
}

QString ApplicationContext::load()
{
    QString source = "users.txt";
    const auto error = [&source] {
        return QString("Không thể nạp %1. Kiểm tra dữ liệu và mở lại ứng dụng.").arg(source);
    };
    try {
        if (!users.load()) {
            return error();
        }
        source = "role_permissions.txt";
        if (!permissions.load()) {
            return error();
        }
    } catch (const std::exception&) {
        return error();
    }

    // Login remains available if a catalog or its references could not be loaded.
    try {
        source = "movies.txt";
        if (!movies.load()) {
            catalogError = error();
            return {};
        }
        source = "rooms.txt";
        if (!rooms.load()) {
            catalogError = error();
            return {};
        }
        source = "seats.txt";
        if (!seats.load()) {
            catalogError = error();
            return {};
        }
        source = "showtimes.txt";
        if (!showtimeRepo.load()) {
            catalogError = error();
        }
    } catch (const std::exception&) {
        catalogError = error();
    }
    return {};
}
