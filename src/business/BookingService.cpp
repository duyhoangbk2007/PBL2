#include "BookingService.h"
#include "../models/User.h"
#include "../models/Room.h"
#include "../models/Member.h"
#include "../models/Invoice.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"

#include <limits>
#include <stdexcept>
using namespace std;

BookingService::BookingService(
    ShowtimeRepository& showtimeRepo,
    SeatRepository& seatRepo,
    TicketRepository& ticketRepo,
    InvoiceRepository& invoiceRepo,
    MemberRepository& memberRepo,
    PricingService& pricingService,
    AuthService& authService
)
    : showtimeRepo(showtimeRepo),
      seatRepo(seatRepo),
      ticketRepo(ticketRepo),
      invoiceRepo(invoiceRepo),
      memberRepo(memberRepo),
      pricingService(pricingService),
      authService(authService)
{
}

vector<Seat*> BookingService::getAvailableSeats(const string& showtimeId)
{
    vector<Seat*> result;
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded())
    {
        return result;
    }
    Showtime* showtime = showtimeRepo.findById(showtimeId);
    if (showtime == nullptr ||
        showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return result;
    }
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), showtime->getStartAt()))
    {
        return result;
    }
    Room* room = showtime->getRoom();
    if (room == nullptr || !room->isActive())
    {
        return result;
    }
    for (int i = 0; i < room->getSeatCount(); i++)
    {
        Seat* seat = room->getSeatAt(i);
        if (seat == nullptr)
        {
            continue;
        }

        if (!ticketRepo.isSeatSold(showtimeId, seat->getSeatId()))
        {
            result.push_back(seat);
        }
    }
    return result;
}
bool BookingService::calculateBooking(const string& showtimeId, const vector<BookingItem>& items, const string& memberId, BookingSummary& summary)
{
    summary = BookingSummary{};
    // Kiem tra cac Repository
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    // Kiem tra suat chieu
    Showtime* showtime = showtimeRepo.findById(showtimeId);
    if (showtime == nullptr ||
        showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), showtime->getStartAt()))
    {
        return false;
    }
    Room* room = showtime->getRoom();
    if (room == nullptr || !room->isActive())
    {
        return false;
    }

    if (items.empty())
    {
        return false;
    }
    // Kiem tra thanh vien
    bool isMember = !memberId.empty();

    if (isMember && memberRepo.findById(memberId) == nullptr)
    {
        return false;
    }

    // Tinh tien tung ve
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        const BookingItem& item = items[i];

        // Kiem tra ghe thuoc phong
        Seat* seat = seatRepo.findById(showtime->getRoomId(), item.seatId);

        if (seat == nullptr)
        {
            return false;
        }

        // Kiem tra ghe da ban
        if (ticketRepo.isSeatSold(showtimeId, item.seatId))
        {
            return false;
        }

        // Kiem tra ghe bi chon trung
        for (int j = 0; j < i; j++)
        {
            if (items[j].seatId == item.seatId)
            {
                return false;
            }
        }

        // Kiem tra loai ve
        if (item.ticketType != TicketType::REGULAR &&
            item.ticketType != TicketType::STUDENT)
        {
            return false;
        }

        // Tinh gia ve
        double basePrice = pricingService.getBasePrice(seat->getSeatType());
        double discount = pricingService.calculateDiscount(basePrice, item.ticketType, isMember);
        double paidAmount = pricingService.calculatePaidAmount(basePrice, discount);

        summary.ticketPrices.push_back(paidAmount);
        summary.totalAmount += paidAmount;
    }

    // Tinh diem thanh vien tren tong tien
    summary.earnedPoints = pricingService.calculatePoints(summary.totalAmount,isMember);
    return true;
}


bool BookingService::createBooking(const string& showtimeId,
                                   const vector<BookingItem>& items,
                                   const string& memberId,
                                   string& invoiceId)
{
    invoiceId = "";
    // 1. Kiem tra quyen
    const User& staff = authService.requirePermission("BOOKING_CREATE");

    // 2. Kiem tra Repository
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded() ||
        !invoiceRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    // 3. Kiem tra du lieu va tinh tien
    BookingSummary summary;
    if (!calculateBooking(showtimeId, items, memberId, summary))
    {
        return false;
    }

    // 4. Lay thong tin giao dich
    string staffId = staff.getUserId();
    string createdAt = DateTime::getCurrentDateTime();
    Showtime* showtime = showtimeRepo.findById(showtimeId);

    Member* member = nullptr;
    if (!memberId.empty())
    {
        member = memberRepo.findById(memberId);
    }

    // 5. Sinh ID hoa don
    vector<string> invoiceIds;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        if (invoice == nullptr)
        {
            return false;
        }
        invoiceIds.push_back(invoice->getInvoiceId());
    }
    string newInvoiceId = IdGenerator::generateNextId("INV", invoiceIds);

    // 6. Lay danh sach ID ve
    vector<string> ticketIds;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        if (ticket == nullptr)
        {
            return false;
        }
        ticketIds.push_back(ticket->getTicketId());
    }

    // Sinh truoc ID cho tat ca ve
    vector<string> newTicketIds;
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        string ticketId = IdGenerator::generateNextId("TKT", ticketIds);
        newTicketIds.push_back(ticketId);
        ticketIds.push_back(ticketId);
    }

    // 7. Kiem tra gioi han diem
    if (member != nullptr &&
        summary.earnedPoints > numeric_limits<int>::max() - member->getPoints())
    {
        return false;
    }

    // 8. Tao Invoice
    Invoice* invoice = new Invoice(newInvoiceId,
                                   showtimeId,
                                   staffId,
                                   memberId,
                                   createdAt,
                                   summary.totalAmount);

    if (!invoiceRepo.add(invoice))
    {
        delete invoice;
        return false;
    }

    // 9. Tao Ticket
    int allocatedPoints = 0;
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        const BookingItem& item = items[i];
        Seat* seat = seatRepo.findById(showtime->getRoomId(), item.seatId);
        double basePrice = pricingService.getBasePrice(seat->getSeatType());
        double paidAmount = summary.ticketPrices[i];
        double discount = basePrice - paidAmount;

        int points = pricingService.calculatePoints(paidAmount, member != nullptr);

        if (i == static_cast<int>(items.size()) - 1)
        {
            points = summary.earnedPoints - allocatedPoints;
        }

        allocatedPoints += points;

        Ticket* ticket = new Ticket(newTicketIds[i],
                                    newInvoiceId,
                                    showtimeId,
                                    item.seatId,
                                    item.ticketType,
                                    basePrice,
                                    discount,
                                    paidAmount,
                                    points,
                                    TicketStatus::SOLD,
                                    "",
                                    "",
                                    "",
                                    0);

        if (!ticketRepo.add(ticket))
        {
            delete ticket;
            return false;
        }
    }

    // 10. Cong diem thanh vien
    if (member != nullptr)
    {
        member->setPoints(member->getPoints() + summary.earnedPoints);
    }

    // 11. Luu du lieu
    if (!invoiceRepo.save() ||
        !ticketRepo.save() ||
        (member != nullptr && !memberRepo.save()))
    {
        return false;
    }

    // 12. Hoan thanh
    invoiceId = newInvoiceId;
    return true;
}






