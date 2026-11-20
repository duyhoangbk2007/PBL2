#include "TicketRepository.h"
#include "ShowtimeRepository.h"
#include "SeatRepository.h"
#include "InvoiceRepository.h"

#include "../models/Showtime.h"
#include "../models/Seat.h"
#include "../models/Invoice.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"

#include <cmath>
#include <stdexcept>
#include <vector>

using namespace std;

TicketRepository::TicketRepository(const string& filePath, ShowtimeRepository& showtimeRepo, SeatRepository& seatRepo, InvoiceRepository& invoiceRepo)
    : repo(filePath, getTicketId, stringToTicket, ticketToString),
      showtimeRepo(showtimeRepo),
      seatRepo(seatRepo),
      invoiceRepo(invoiceRepo)
{}

string TicketRepository::getTicketId(Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return "";
    }
    return ticket->getTicketId();
}

bool TicketRepository::isValidTicket(const Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(ticket->getTicketId(), "TKT") ||
        !Validator::isValidId(ticket->getInvoiceId(), "INV") ||
        !Validator::isValidId(ticket->getShowtimeId(), "SHW"))
    {
        return false;
    }
    if (!Validator::isValidText(ticket->getSeatId()))
    {
        return false;
    }
    TicketType type = ticket->getTicketType();
    if (type != TicketType::REGULAR &&
        type != TicketType::STUDENT)
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
        !isfinite(paid) || !isfinite(refund))
    {
        return false;
    }
    if (base < 0 || discount < 0 ||
        paid < 0 || refund < 0 ||
        ticket->getPointsEarned() < 0)
    {
        return false;
    }

    if (discount > base ||
        fabs(paid - (base - discount)) > 0.000001)
    {
        return false;
    }
    // Vé đã bán rồi thì không được hủy
    if (status == TicketStatus::SOLD)
    {
        if (!ticket->getCancelledAt().empty() ||
            !ticket->getCancelledBy().empty() ||
            !ticket->getCancelReason().empty() ||
            refund != 0)
        {
            return false;
        }
    }
    else
    {
        if (!Validator::isValidDateTime(ticket->getCancelledAt()) ||
            !Validator::isValidId(ticket->getCancelledBy(), "USR") ||
            !Validator::isValidText(ticket->getCancelReason()) ||
            fabs(refund - paid) > 0.000001)
        {
            return false;
        }
    }

    return true;
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
        size_t p1 = 0, p2 = 0, p3 = 0;
        size_t p4 = 0, p5 = 0;
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

bool TicketRepository::isValidReference(const Ticket* ticket)
{
    if (!isValidTicket(ticket))
    {
        return false;
    }
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
    Seat* seat = seatRepo.findById(showtime->getRoomId(), ticket->getSeatId());
    if (seat == nullptr)
    {
        return false;
    }
    if (invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        return false;
    }
    if (ticket->getStatus() == TicketStatus::CANCELLED)
    {
        // hủy vé trước giờ chiếu
        if (!DateTime::isBefore(ticket->getCancelledAt(), showtime->getStartAt()))
        {
            return false;
        }
        // thời gian hủy vé trước thời gian tạo hóa đơn là vô lý
        if (DateTime::isBefore(ticket->getCancelledAt(), invoice->getCreatedAt()))
        {
            return false;
        }
    }

    return true;
}

bool TicketRepository::load()
{
    if (repo.isLoaded() ||
        !showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !invoiceRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    // Kiem tra toan bo du lieu truoc khi lien ket
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);
        if (!isValidReference(ticket))
        {
            repo.clear();
            return false;
        }
        // nếu vé đang xét là CANCELLED thì bỏ qua
        if (ticket->getStatus() != TicketStatus::SOLD)
        {
            continue;
        }
        // kiểm tra trùng
        for (int j = 0; j < i; j++)
        {
            Ticket* other = repo.getAt(j);
            if (other->getStatus() == TicketStatus::SOLD &&
                other->getShowtimeId() == ticket->getShowtimeId() &&
                other->getSeatId() == ticket->getSeatId())
            {
                repo.clear();
                return false;
            }
        }
    }

    // Lien ket Ticket voi cac doi tuong tham chieu
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);
        Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
        Seat* seat = seatRepo.findById(showtime->getRoomId(), ticket->getSeatId());
        Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

        if (!ticket->linkShowtime(showtime) ||
            !ticket->linkSeat(seat) ||
            !ticket->linkInvoice(invoice))
        {
            // Thao cac lien ket Ticket-Invoice da tao
            for (int j = 0; j <= i; j++)
            {
                Ticket* linked = repo.getAt(j);
                Invoice* owner = invoiceRepo.findById(linked->getInvoiceId());
                if (owner != nullptr &&
                    linked->getInvoice() == owner)
                {
                    owner->unlinkTicket(linked);
                }
            }
            repo.clear();
            return false;
        }
    }
    return true;
}

bool TicketRepository::save()
{
    return repo.save();
}

bool TicketRepository::add(Ticket* ticket)
{
    if (!repo.isLoaded() || !isValidReference(ticket))
    {
        return false;
    }
    if (repo.findById(ticket->getTicketId()) != nullptr)
    {
        return false;
    }
    // có rồi thì không thêm nữa
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
    if (ticket->getStatus() == TicketStatus::SOLD &&
        isSeatSold(ticket->getShowtimeId(), ticket->getSeatId()))
    {
        return false;
    }
    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    Seat* seat = seatRepo.findById(showtime->getRoomId(), ticket->getSeatId());
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

    if (!ticket->linkShowtime(showtime) ||
        !ticket->linkSeat(seat))
    {
        return false;
    }
    if (!repo.add(ticket))
    {
        return false;
    }
    if (!ticket->linkInvoice(invoice))
    {
        // Ticket chua duoc lien ket Invoice,
        // nen co the xoa khoi Repository.
        repo.removeById(ticket->getTicketId());
        return false;
    }
    return true;
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

vector<Ticket*> TicketRepository::getByShowtime(const string& showtimeId)
{
    vector<Ticket*> result;
    if (!repo.isLoaded())
    {
        return result;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket->getShowtimeId() == showtimeId)
        {
            result.push_back(ticket);
        }
    }
    return result;
}

vector<Ticket*> TicketRepository::getByInvoice(const string& invoiceId)
{
    vector<Ticket*> result;
    if (!repo.isLoaded())
    {
        return result;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket->getInvoiceId() == invoiceId)
        {
            result.push_back(ticket);
        }
    }
    return result;
}

bool TicketRepository::isSeatSold(const string& showtimeId, const string& seatId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
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
