
#include "ApplicationContext.h"

#include <QDir>
#include <QStringList>
#include <exception>
#include <functional>
#include <vector>

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

QString ApplicationContext::saveAll()
{
    struct SaveStep {
        QString fileName;
        bool loaded;
        std::function<bool()> save;
    };

    // Dependencies are written before the records that refer to them.
    const std::vector<SaveStep> steps = {
        {"users.txt", users.isLoaded(), [this] { return users.save(); }},
        {"movies.txt", movieRepo.isLoaded(), [this] { return movieRepo.save(); }},
        {"rooms.txt", roomRepo.isLoaded(), [this] { return roomRepo.save(); }},
        {"seats.txt", seatRepo.isLoaded(), [this] { return seatRepo.save(); }},
        {"showtimes.txt", showtimeRepo.isLoaded(), [this] { return showtimeRepo.save(); }},
        {"pricing.txt", pricingRepo.isLoaded(), [this] { return pricingRepo.save(); }},
        {"members.txt", memberRepo.isLoaded(), [this] { return memberRepo.save(); }},
        {"invoices.txt", invoiceRepo.isLoaded(), [this] { return invoiceRepo.save(); }},
        {"tickets.txt", ticketRepo.isLoaded(), [this] { return ticketRepo.save(); }}
    };

    QStringList missing;
    for (const SaveStep& step : steps) {
        if (!step.loaded) {
            missing << step.fileName;
        }
    }
    if (!missing.isEmpty()) {
        return "Chua nap du lieu: " + missing.join(", ") + ". Chua ghi file nao.";
    }

    QStringList saved;
    for (const SaveStep& step : steps) {
        try {
            if (step.save()) {
                saved << step.fileName;
                continue;
            }
        } catch (const std::exception& error) {
            return "Luu " + step.fileName + " that bai: " +
                   QString::fromUtf8(error.what()) + ". Da ghi: " + saved.join(", ") + ".";
        } catch (...) {
            return "Luu " + step.fileName + " that bai. Da ghi: " + saved.join(", ") + ".";
        }
        return "Luu " + step.fileName + " that bai. Da ghi: " + saved.join(", ") + ".";
    }
    return {};
}
