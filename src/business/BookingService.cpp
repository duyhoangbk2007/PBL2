#include "BookingService.h"
#include "../models/User.h"
#include "../models/Room.h"
#include "../models/Member.h"
#include "../models/Invoice.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"

#include <limits>
#include <cmath>
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
    BookingSummary calculated;
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
        int points = pricingService.calculatePoints(paidAmount, isMember);
        if (!isfinite(paidAmount) || paidAmount < 0 ||
            !isfinite(calculated.totalAmount + paidAmount) ||
            points > numeric_limits<int>::max() - calculated.earnedPoints)
        {
            return false;
        }
        calculated.ticketPrices.push_back(paidAmount);
        calculated.totalAmount += paidAmount;
        calculated.earnedPoints += points;
    }
    summary = calculated;
    return true;
}

bool BookingService::createBooking(const string& showtimeId,
                                   const vector<BookingItem>& items,
                                   const string& memberId,
                                   bool ageConfirmed,
                                   bool paymentConfirmed,
                                   string& invoiceId)
{
    invoiceId = "";
    const User& staff = authService.requirePermission("TICKET_SELL");
    if (storageFailed)
    {
        throw runtime_error("Giao dich truoc ghi TXT loi; can kiem tra du lieu truoc khi ban tiep");
    }
    if (!ageConfirmed || !paymentConfirmed)
    {
        return false;
    }

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
    if (member != nullptr && summary.earnedPoints > numeric_limits<int>::max() - member->getPoints())
    {
        return false;
    }

    vector<double> basePrices; // lưu giá gốc
    vector<int> ticketPoints;
    for (size_t i = 0; i < items.size(); i++)
    {
        Seat* seat = seatRepo.findById(showtime->getRoomId(), items[i].seatId);
        if (seat == nullptr)
        {
            return false;
        }
        basePrices.push_back(pricingService.getBasePrice(seat->getSeatType()));
        ticketPoints.push_back(pricingService.calculatePoints(summary.ticketPrices[i], member != nullptr));
    }

    vector<string> addedTicketIds; // theo dõi các id vừa thêm
    addedTicketIds.reserve(items.size()); //thêm dung lượng cho vector
    bool invoiceAdded = false; // Đánh dấu Invoice đã được thêm hay chưa
    bool ticketAddFailed = false; // vé thêm vào bị lỗi
    int oldPoints = member == nullptr ? 0 : member->getPoints();
    try
    {
        Invoice* invoice = new Invoice(newInvoiceId, showtimeId, staffId, memberId, createdAt, summary.totalAmount);
        try
        {
            if (!invoiceRepo.add(invoice))
            {
                delete invoice;
                return false;
            }
        }
        catch (...)
        {
            delete invoice;
            throw;
        }
        invoiceAdded = true;
        // Tạo từng vé
        for (size_t i = 0; i < items.size(); i++)
        {
            double paidAmount = summary.ticketPrices[i];
            addedTicketIds.push_back(newTicketIds[i]);
            Ticket* ticket = nullptr;
            try
            {
                ticket = new Ticket(newTicketIds[i], newInvoiceId, showtimeId,
                                    items[i].seatId, items[i].ticketType,
                                    basePrices[i], basePrices[i] - paidAmount,
                                    paidAmount, ticketPoints[i], TicketStatus::SOLD,
                                    "", "", "", 0);
                if (!ticketRepo.add(ticket))
                {
                    delete ticket;
                    addedTicketIds.pop_back();
                    ticketAddFailed = true;
                    break;
                }
            }
            catch (...)
            {
                delete ticket;
                addedTicketIds.pop_back();
                throw;
            }
        }
        if (!ticketAddFailed && member != nullptr)
        {
            member->setPoints(oldPoints + summary.earnedPoints);
        }
    }
    catch (...)
    {
        rollbackBooking(addedTicketIds,newInvoiceId, invoiceAdded, member, oldPoints);
        throw;
    }
    if (ticketAddFailed)
    {
        rollbackBooking(addedTicketIds,newInvoiceId, invoiceAdded, member, oldPoints);
        return false;
    }

    // Tu lan ghi dau tien, file co the da thay doi du save() tra false.
    try
    {
        if (!invoiceRepo.save())
        {
            throw runtime_error("Khong the ghi file Invoice");
        }
        if (!ticketRepo.save())
        {
            throw runtime_error("Khong the ghi file Ticket; Invoice co the da duoc luu");
        }
        if (member != nullptr && !memberRepo.save())
        {
            throw runtime_error("Khong the ghi file Member; Invoice/Ticket co the da duoc luu");
        }
    }
    catch (...)
    {
        storageFailed = true;
        throw;
    }
    invoiceId = newInvoiceId;
    return true;
}

void BookingService::rollbackBooking(const vector<string>& addedTicketIds,
                                     const string& invoiceId,
                                     bool invoiceAdded,
                                     Member* member,
                                     int oldPoints)
{
    // Khoi phuc diem thanh vien
    if (member != nullptr)
    {
        member->setPoints(oldPoints);
    }
    // Rollback Ticket theo thu tu nguoc
    for (int i = static_cast<int>(addedTicketIds.size()) - 1; i >= 0; i--)
    {
        if (!ticketRepo.rollbackPending(addedTicketIds[i]))
        {
            throw runtime_error("Khong the rollback Ticket trong RAM");
        }
    }
    // Rollback Invoice sau khi da xoa het Ticket
    if (invoiceAdded && !invoiceRepo.rollbackPending(invoiceId))
    {
        throw runtime_error("Khong the rollback Invoice trong RAM");
    }
}






