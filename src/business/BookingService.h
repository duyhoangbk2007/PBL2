#ifndef BOOKING_SERVICE_H
#define BOOKING_SERVICE_H

#include <string>
#include <vector>
#include <stack>

#include "../models/Seat.h"
#include "../models/Ticket.h"

#include "../data_access/ShowtimeRepository.h"
#include "../data_access/SeatRepository.h"
#include "../data_access/TicketRepository.h"
#include "../data_access/InvoiceRepository.h"
#include "../data_access/MemberRepository.h"

#include "PricingService.h"
#include "AuthService.h"

struct BookingItem //một ghế được chọn cùng loại vé
{
    std::string seatId;
    TicketType ticketType;
};

struct BookingSummary //kết quả tính tiền trước khi khách xác nhận.
{ 
    std::vector<double> ticketPrices; //danh sách tiền của từng ghế khách mua
    double totalAmount = 0;
    int earnedPoints = 0;
};

enum class RollbackType
{
    ADD_INVOICE,
    ADD_TICKET,
    UPDATE_MEMBER_POINTS
};

struct RollbackAction
{
    RollbackType type;
    std::string id;
    int oldPoints = 0;
};

class BookingService
{
private:
    ShowtimeRepository& showtimeRepo;
    SeatRepository& seatRepo;
    TicketRepository& ticketRepo;
    InvoiceRepository& invoiceRepo;
    MemberRepository& memberRepo;

    PricingService& pricingService;
    AuthService& authService;

    bool rollback(std::stack<RollbackAction>& actions);

public:
    BookingService(ShowtimeRepository& showtimeRepo,
                   SeatRepository& seatRepo,
                   TicketRepository& ticketRepo,
                   InvoiceRepository& invoiceRepo,
                   MemberRepository& memberRepo,
                   PricingService& pricingService,
                   AuthService& authService);

    std::vector<Seat*> getAvailableSeats(const std::string& showtimeId);

    bool calculateBooking(const std::string& showtimeId, const std::vector<BookingItem>& items, const std::string& memberId, BookingSummary& summary);

    bool createBooking(const std::string& showtimeId, const std::vector<BookingItem>& items, const std::string& memberId, std::string& invoiceId);
};

#endif
