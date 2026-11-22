
#include "ApplicationContext.h"

#include <QDir>
#include <exception>

ApplicationContext::ApplicationContext(
    const QString& dataDirectory)
    : users(
          QDir(dataDirectory)
              .filePath("users.txt").toStdString()),
      permissions(
          QDir(dataDirectory)
              .filePath("role_permissions.txt").toStdString()),

      movieRepo(
          QDir(dataDirectory)
              .filePath("movies.txt").toStdString()),
      roomRepo(
          QDir(dataDirectory)
              .filePath("rooms.txt").toStdString()),
      seatRepo(
          QDir(dataDirectory)
              .filePath("seats.txt").toStdString(),
          roomRepo),
      showtimeRepo(
          QDir(dataDirectory)
              .filePath("showtimes.txt").toStdString(),
          movieRepo, roomRepo),
      memberRepo(
          QDir(dataDirectory)
              .filePath("members.txt").toStdString()),
      pricingRepo(
          QDir(dataDirectory)
              .filePath("pricing.txt").toStdString()),

      invoiceRepo(
          QDir(dataDirectory)
              .filePath("invoices.txt").toStdString(),
          showtimeRepo, users, memberRepo),
      ticketRepo(
          QDir(dataDirectory)
              .filePath("tickets.txt").toStdString(),
          showtimeRepo, seatRepo, invoiceRepo),

      auth(users, permissions),
      movies(movieRepo, auth),
      rooms(roomRepo, auth),
      seats(seatRepo, auth),
      showtimes(
          showtimeRepo, movieRepo, roomRepo, auth),
      staff(users, auth),
      members(memberRepo, auth),
      pricing(pricingRepo, auth),

      booking(
          showtimeRepo, seatRepo,
          ticketRepo, invoiceRepo,
          memberRepo, pricing, auth),
      invoices(invoiceRepo, ticketRepo, auth),
      reports(
          invoiceRepo, ticketRepo,
          movieRepo, showtimeRepo, auth)
{
}

QString ApplicationContext::load()
{
    pricingError.clear();
    memberError.clear();
    catalogError.clear();
    transactionError.clear();

    QString source = "users.txt";

    const auto error = [&source] {
        return QString(
            "Không thể nạp %1. "
            "Kiểm tra dữ liệu và mở lại ứng dụng."
        ).arg(source);
    };

    // 1. Dữ liệu đăng nhập và phân quyền
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

    // 2. Cấu hình giá
    source = "pricing.txt";
    try {
        if (!pricing.load()) {
            pricingError = error();
        }
    } catch (const std::exception&) {
        pricingError = error();
    }

    // 3. Thành viên
    source = "members.txt";
    try {
        if (!members.load()) {
            memberError = error();
        }
    } catch (const std::exception&) {
        memberError = error();
    }

    // 4. Danh mục phim, phòng, ghế, suất chiếu
    // Giữ khả năng đăng nhập khi danh mục lỗi.
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
        if (!showtimes.load()) {
            catalogError = error();
            return {};
        }
    } catch (const std::exception&) {
        catalogError = error();
        return {};
    }

    // 5. Dữ liệu giao dịch
    // Invoice phải load trước Ticket.
    try {
        source = "invoices.txt";
        if (!invoiceRepo.load()) {
            transactionError = error();
            return {};
        }

        source = "tickets.txt";
        if (!ticketRepo.load()) {
            transactionError = error();
            return {};
        }
    } catch (const std::exception&) {
        transactionError = error();
    }

    return {};
}
