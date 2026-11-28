#include "TicketRepository.h"
#include "../models/Showtime.h"
#include "../models/Seat.h"
#include "../models/Invoice.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <cmath>
#include <stdexcept>
#include <vector>
using namespace std;

bool TicketRepository::isValidTicket(const Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return false;
    }

    if (!Validator::isValidId(ticket->getTicketId(), "TKT") ||
        !Validator::isValidId(ticket->getInvoiceId(), "INV") ||
        !Validator::isValidId(ticket->getShowtimeId(), "SHW") ||
        !Validator::isValidText(ticket->getSeatId()))
    {
        return false;
    }

    if (ticket->getTicketType() != TicketType::REGULAR &&
        ticket->getTicketType() != TicketType::STUDENT)
    {
        return false;
    }

    TicketStatus status = ticket->getStatus();

    if (status != TicketStatus::SOLD &&
        status != TicketStatus::CANCELLED)
    {
        return false;
    }

    double base = ticket->getBasePrice();
    double discount = ticket->getDiscountAmount();
    double paid = ticket->getPaidAmount();
    double refund = ticket->getRefundAmount();

    if (!isfinite(base) || !isfinite(discount) ||
        !isfinite(paid) || !isfinite(refund) ||
        base < 0 || discount < 0 ||
        paid < 0 || refund < 0 ||
        ticket->getPointsEarned() < 0 ||
        discount > base ||
        fabs(paid - (base - discount)) > 0.000001)
    {
        return false;
    }

    if (status == TicketStatus::SOLD)
    {
        return ticket->getCancelledAt().empty() &&
               ticket->getCancelledBy().empty() &&
               ticket->getCancelReason().empty() &&
               refund == 0;
    }

    return Validator::isValidDateTime(ticket->getCancelledAt()) &&
           Validator::isValidId(ticket->getCancelledBy(), "USR") &&
           Validator::isValidText(ticket->getCancelReason()) &&
           fabs(refund - paid) <= 0.000001;
}

TicketRepository::TicketRepository(const string& filePath,
                                     IRepository<Showtime>& showtimeRepo,
                                     IRepository<Seat>& seatRepo,
                                     IRepository<Invoice>& invoiceRepo)
    : repo(filePath, getTicketId, stringToTicket, ticketToString),
      showtimeRepo(showtimeRepo),
      seatRepo(seatRepo),
      invoiceRepo(invoiceRepo)
{
}

string TicketRepository::getTicketId(Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return "";
    }
    return ticket->getTicketId();
}

Ticket* TicketRepository::stringToTicket(const string& line)
{
    vector<string> fields;
    size_t start = 0;

    while (true)
    {
        size_t pos = line.find('|', start);

        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }

        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    if (fields.size() != 14)
    {
        throw runtime_error("Sai so luong truong Ticket");
    }

    TicketType type;

    if (fields[4] == "REGULAR")
    {
        type = TicketType::REGULAR;
    }
    else if (fields[4] == "STUDENT")
    {
        type = TicketType::STUDENT;
    }
    else
    {
        throw runtime_error("Loai ve khong hop le");
    }

    TicketStatus status;

    if (fields[9] == "SOLD")
    {
        status = TicketStatus::SOLD;
    }
    else if (fields[9] == "CANCELLED")
    {
        status = TicketStatus::CANCELLED;
    }
    else
    {
        throw runtime_error("Trang thai ve khong hop le");
    }

    double basePrice;
    double discountAmount;
    double paidAmount;
    double refundAmount;
    int pointsEarned;

    try
    {
        int indexes[] = {5, 6, 7, 8, 13};

        for (int index : indexes)
        {
            if (!Validator::isValidNumber(fields[index]))
            {
                throw runtime_error("Sai dinh dang so");
            }
        }

        size_t p1 = 0;
        size_t p2 = 0;
        size_t p3 = 0;
        size_t p4 = 0;
        size_t p5 = 0;

        basePrice = stod(fields[5], &p1);
        discountAmount = stod(fields[6], &p2);
        paidAmount = stod(fields[7], &p3);
        pointsEarned = stoi(fields[8], &p4);
        refundAmount = stod(fields[13], &p5);

        if (p1 != fields[5].size() ||
            p2 != fields[6].size() ||
            p3 != fields[7].size() ||
            p4 != fields[8].size() ||
            p5 != fields[13].size())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Gia tien hoac diem Ticket khong hop le");
    }

    Ticket* ticket = new Ticket(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        type,
        basePrice,
        discountAmount,
        paidAmount,
        pointsEarned,
        status,
        fields[10],
        fields[11],
        fields[12],
        refundAmount
    );

    if (!isValidTicket(ticket))
    {
        delete ticket;
        throw runtime_error("Du lieu Ticket khong hop le");
    }

    return ticket;
}

string TicketRepository::ticketToString(Ticket* ticket)
{
    if (!isValidTicket(ticket))
    {
        throw runtime_error("Ticket khong hop le khi ghi file");
    }

    string type = ticket->getTicketType() == TicketType::REGULAR
        ? "REGULAR" : "STUDENT";

    string status = ticket->getStatus() == TicketStatus::SOLD
        ? "SOLD" : "CANCELLED";

    return ticket->getTicketId() + "|" +
           ticket->getInvoiceId() + "|" +
           ticket->getShowtimeId() + "|" +
           ticket->getSeatId() + "|" +
           type + "|" +
           to_string(ticket->getBasePrice()) + "|" +
           to_string(ticket->getDiscountAmount()) + "|" +
           to_string(ticket->getPaidAmount()) + "|" +
           to_string(ticket->getPointsEarned()) + "|" +
           status + "|" +
           ticket->getCancelledAt() + "|" +
           ticket->getCancelledBy() + "|" +
           ticket->getCancelReason() + "|" +
           to_string(ticket->getRefundAmount());
}

TicketRepository::~TicketRepository()
{
    unlinkAll();
}

bool TicketRepository::isValidReference(const Ticket* ticket)
{
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !invoiceRepo.isLoaded())
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

    if (showtime == nullptr || invoice == nullptr)
    {
        return false;
    }

    Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                   ticket->getSeatId());
    if (seat == nullptr ||
        invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        return false;
    }

    if (ticket->getPointsEarned() > 0 && invoice->getMember() == nullptr)
    {
        return false;
    }

    if (ticket->getStatus() == TicketStatus::CANCELLED)
    {
        if (!DateTime::isBefore(ticket->getCancelledAt(),
                                showtime->getStartAt()) ||
            DateTime::isBefore(ticket->getCancelledAt(),
                               invoice->getCreatedAt()))
        {
            return false;
        }
    }

    return true;
}

bool TicketRepository::isSeatSold(const string& showtimeId,
                                  const string& seatId)
{
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket->getShowtimeId() == showtimeId &&
            ticket->getSeatId() == seatId &&
            ticket->getStatus() == TicketStatus::SOLD)
        {
            return true;
        }
    }

    return false;
}

void TicketRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket != nullptr)
        {
            ticket->unlinkReferences();
        }
    }
}

bool TicketRepository::load()
{
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !invoiceRepo.isLoaded())
    {
        return false;
    }

    if (!repo.load())
    {
        return false;
    }

    try
    {
        // Kiem tra toan bo du lieu truoc khi lien ket
        for (int i = 0; i < repo.count(); i++)
        {
            Ticket* ticket = repo.getAt(i);

            if (!isValidReference(ticket))
            {
                throw runtime_error(
                    "Ticket co tham chieu khong hop le"
                );
            }

            if (ticket->getStatus() != TicketStatus::SOLD)
            {
                continue;
            }

            for (int j = 0; j < i; j++)
            {
                Ticket* other = repo.getAt(j);

                if (other->getStatus() == TicketStatus::SOLD &&
                    other->getShowtimeId() == ticket->getShowtimeId() &&
                    other->getSeatId() == ticket->getSeatId())
                {
                    throw runtime_error("Trung ghe da ban");
                }
            }
        }

        // Lien ket sau khi tat ca ban ghi hop le
        for (int i = 0; i < repo.count(); i++)
        {
            Ticket* ticket = repo.getAt(i);

            Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
            Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                           ticket->getSeatId());
            Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

            if (!ticket->linkShowtime(showtime) ||
                !ticket->linkSeat(seat) ||
                !ticket->linkInvoice(invoice))
            {
                throw runtime_error(
                    "Khong the lien ket Ticket"
                );
            }
        }
    }
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }

    return true;
}

bool TicketRepository::save()
{
    return repo.save();
}

bool TicketRepository::add(Ticket* ticket)
{
    if (!repo.isLoaded() || !isValidTicket(ticket))
    {
        return false;
    }

    if (repo.findById(ticket->getTicketId()) != nullptr)
    {
        return false;
    }

    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == ticket)
        {
            return false;
        }
    }

    if (ticket->getShowtime() != nullptr ||
        ticket->getSeat() != nullptr ||
        ticket->getInvoice() != nullptr)
    {
        return false;
    }
    if (!isValidReference(ticket))
    {
        return false;
    }

    if (ticket->getStatus() == TicketStatus::SOLD &&
        isSeatSold(ticket->getShowtimeId(), ticket->getSeatId()))
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                   ticket->getSeatId());
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

    try
    {
        if (!ticket->linkShowtime(showtime) ||
            !ticket->linkSeat(seat) ||
            !ticket->linkInvoice(invoice))
        {
            ticket->unlinkReferences();
            return false;
        }

        if (!repo.add(ticket))
        {
            ticket->unlinkReferences();
            return false;
        }
    }
    catch (...)
    {
        ticket->unlinkReferences();
        throw;
    }

    return true;
}

bool TicketRepository::update(const Ticket& newValues)
{
    if (!repo.isLoaded() || !isValidTicket(&newValues))
    {
        return false;
    }

    Ticket* ticket = repo.findById(newValues.getTicketId());

    if (ticket == nullptr ||
        ticket->getStatus() != TicketStatus::SOLD ||
        newValues.getStatus() != TicketStatus::CANCELLED)
    {
        return false;
    }

    // Chi duoc thay doi thong tin huy ve
    if (ticket->getInvoiceId() != newValues.getInvoiceId() ||
        ticket->getShowtimeId() != newValues.getShowtimeId() ||
        ticket->getSeatId() != newValues.getSeatId() ||
        ticket->getTicketType() != newValues.getTicketType() ||
        ticket->getBasePrice() != newValues.getBasePrice() ||
        ticket->getDiscountAmount() != newValues.getDiscountAmount() ||
        ticket->getPaidAmount() != newValues.getPaidAmount() ||
        ticket->getPointsEarned() != newValues.getPointsEarned())
    {
        return false;
    }

    if (!isValidReference(&newValues))
    {
        return false;
    }

    ticket->markCancelled(
        newValues.getCancelledAt(),
        newValues.getCancelledBy(),
        newValues.getCancelReason()
    );

    return true;
}

bool TicketRepository::removeById(const string& ticketId)
{
    if (!repo.isLoaded())
    {
        return false;
    }

    Ticket* ticket = repo.findById(ticketId);

    if (ticket == nullptr)
    {
        return false;
    }

    ticket->unlinkReferences();

    return repo.removeById(ticketId);
}

Ticket* TicketRepository::findById(const string& ticketId)
{
    return repo.findById(ticketId);
}

Ticket* TicketRepository::getAt(int index)
{
    return repo.getAt(index);
}

int TicketRepository::count()
{
    return repo.count();
}

bool TicketRepository::isLoaded()
{
    return repo.isLoaded();
}
