
#pragma once

#include "../business/AuthService.h"
#include "../business/MovieService.h"
#include "../business/RoomService.h"
#include "../business/SeatService.h"
#include "../business/ShowtimeService.h"
#include "../business/StaffService.h"
#include "../business/MemberService.h"
#include "../business/PricingService.h"

#include "../business/BookingService.h"
#include "../business/InvoiceService.h"
#include "../business/ReportService.h"

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
    MemberRepository memberRepo;
    PricingRepository pricingRepo;

    InvoiceRepository invoiceRepo;
    TicketRepository ticketRepo;

public:
    explicit ApplicationContext(const QString& dataDirectory);

    QString load();

    AuthService auth;
    MovieService movies;
    RoomService rooms;
    SeatService seats;
    ShowtimeService showtimes;
    StaffService staff;
    MemberService members;
    PricingService pricing;

    BookingService booking;
    InvoiceService invoices;
    ReportService reports;

    QString pricingError;
    QString memberError;
    QString catalogError;
    QString transactionError;
};
