#ifndef BOOKING_SERVICE_H
#define BOOKING_SERVICE_H

#include <string>
#include <vector>

#include "../models/Seat.h"
#include "../models/Ticket.h"

#include "../data_access/ShowtimeRepository.h"
#include "../data_access/SeatRepository.h"
#include "../data_access/TicketRepository.h"
#include "../data_access/InvoiceRepository.h"
#include "../data_access/MemberRepository.h"

#include "PricingService.h"
#include "AuthService.h"

struct BookingItem
{
    std::string seatId;
    TicketType ticketType;
};

struct BookingSummary
{
    std::vector<double> ticketPrices; //lưu giá phải trả của 1 vé
    double totalAmount = 0; // tổng tiền
    int earnedPoints = 0;
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
    
    bool storageFailed = false;
    void rollbackBooking(const std::vector<std::string>& addedTicketIds,
                         const std::string& invoiceId,
                         bool invoiceAdded,
                         Member* member,
                         int oldPoints);
public:
    BookingService(ShowtimeRepository& showtimeRepo,
                   SeatRepository& seatRepo,
                   TicketRepository& ticketRepo,
                   InvoiceRepository& invoiceRepo,
                   MemberRepository& memberRepo,
                   PricingService& pricingService,
                   AuthService& authService);

    std::vector<Seat*> getAvailableSeats(const std::string& showtimeId);

    bool calculateBooking(const std::string& showtimeId,
                          const std::vector<BookingItem>& items,
                          const std::string& memberId,
                          BookingSummary& summary);

    bool createBooking(const std::string& showtimeId,
                       const std::vector<BookingItem>& items,
                       const std::string& memberId,
                       bool ageConfirmed,
                       bool paymentConfirmed,
                       std::string& invoiceId);

    bool cancelTicket(const std::string& ticketId,
                      const std::string& reason,
                      bool refundConfirmed);
};

#endif
