#include "ReportService.h"
#include "../utils/DateTime.h"
#include "../utils/Validator.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
using namespace std;


ReportService::ReportService(InvoiceRepository& invoiceRepo,
                             TicketRepository& ticketRepo,
                             MovieRepository& movieRepo,
                             ShowtimeRepository& showtimeRepo,
                             AuthService& authService)
    : invoiceRepo(invoiceRepo),
      ticketRepo(ticketRepo),
      movieRepo(movieRepo),
      showtimeRepo(showtimeRepo),
      authService(authService)
{
}

void ReportService::requireReportAccess() const
{
    authService.requireAdmin();
    authService.requirePermission("REPORT_VIEW");
}

void ReportService::validateRange(const string& from, const string& to) const
{
    if (!Validator::isValidDateTime(from) ||
        !Validator::isValidDateTime(to) ||
        !DateTime::isBefore(from, to))
    {
        throw invalid_argument("Khoang thoi gian bao cao khong hop le");
    }
}

void ReportService::validateInvoice(const Invoice* invoice) const
{
    if (invoice == nullptr ||
        !Validator::isValidDateTime(invoice->getCreatedAt()) ||
        !isfinite(invoice->getTotalAmount()) ||
        invoice->getTotalAmount() < 0)
    {
        throw runtime_error("Du lieu hoa don bao cao khong hop le");
    }
}

// trả về invoice tương ứng với Ticket
Invoice* ReportService::requireInvoiceForTicket(Ticket* ticket)
{
    // kiểm tra ticket và số tiền
    if (ticket == nullptr ||
        !isfinite(ticket->getPaidAmount()) ||
        ticket->getPaidAmount() < 0 ||
        !isfinite(ticket->getRefundAmount()) ||
        ticket->getRefundAmount() < 0)
    {
        throw runtime_error("Du lieu ve bao cao khong hop le");
    }

    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());
    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    // kiểm tra liên kết
    if (invoice == nullptr || showtime == nullptr ||
        ticket->getInvoice() != invoice ||
        ticket->getShowtime() != showtime ||
        invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        throw runtime_error("Lien ket ve, hoa don hoac suat chieu khong hop le");
    }

    validateInvoice(invoice);

    // Kiểm tra vé đang SOLD
    if (ticket->getStatus() == TicketStatus::SOLD)
    {
        if (ticket->getRefundAmount() != 0 ||
            !ticket->getCancelledAt().empty() ||
            !ticket->getCancelledBy().empty() ||
            !ticket->getCancelReason().empty())
        {
            throw runtime_error("Ve dang ban co du lieu huy");
        }
    }
    else if (ticket->getStatus() == TicketStatus::CANCELLED)
    {
        if (!Validator::isValidDateTime(ticket->getCancelledAt()) ||
            !Validator::isValidId(ticket->getCancelledBy(), "USR") ||
            !Validator::isValidText(ticket->getCancelReason()) ||
            !DateTime::isBefore(ticket->getCancelledAt(), showtime->getStartAt()) ||
            DateTime::isBefore(ticket->getCancelledAt(), invoice->getCreatedAt()) ||
            fabs(ticket->getRefundAmount() - ticket->getPaidAmount()) > 0.000001)
        {
            throw runtime_error("Du lieu huy ve khong hop le");
        }
    }
    else
    {
        throw runtime_error("Trang thai ve khong hop le");
    }

    return invoice;
}

// tính tổng doanh thu của 1 phim trong 1 khoảng tgian
double ReportService::calculateMovieRevenue(const string& movieId, const string& from, const string& to, bool& hasSoldTicket)
{
    hasSoldTicket = false;
    double revenue = 0;

    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        Invoice* invoice = requireInvoiceForTicket(ticket);

        const Showtime* showtime = ticket->getShowtime();
        Movie* movie = movieRepo.findById(showtime->getMovieId());

        if (movie == nullptr || showtime->getMovie() != movie)
        {
            throw runtime_error("Suat chieu khong lien ket phim hop le");
        }

        if (ticket->getStatus() != TicketStatus::SOLD ||
            showtime->getMovieId() != movieId ||
            invoice->getCreatedAt() < from ||
            invoice->getCreatedAt() >= to)
        {
            continue;
        }

        hasSoldTicket = true;
        revenue += ticket->getPaidAmount();

        if (!isfinite(revenue))
        {
            throw runtime_error("Doanh thu phim khong hop le");
        }
    }

    return revenue;
}

// pair cho phép lưu 2 giá trị trong 1 đối tượng
bool ReportService::compareMovieRevenue(const pair<Movie*, double>& movieA,
                                        const pair<Movie*, double>& movieB)
{
    if (movieA.second != movieB.second)
    {
        return movieA.second > movieB.second;
    }
    return movieA.first->getMovieId() < movieB.first->getMovieId();
}

// tính tổng tiền bán vé trong 1 khoảng thời gian
double ReportService::getTotalSales(const string& from, const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
    double total = 0;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        validateInvoice(invoice);
        if (invoice->getCreatedAt() >= from && invoice->getCreatedAt() < to)
        {
            total += invoice->getTotalAmount();
            if (!isfinite(total))
            {
                throw runtime_error("Tong tien ban khong hop le");
            }
        }
    }
    return total;
}

double ReportService::getTotalRefund(const string& from,
                                     const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
    double totalRefund = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        requireInvoiceForTicket(ticket);
        if (ticket->getStatus() != TicketStatus::CANCELLED ||
            ticket->getCancelledAt() < from ||
            ticket->getCancelledAt() >= to)
        {
            continue;
        }
        totalRefund += ticket->getRefundAmount();
        if (!isfinite(totalRefund))
        {
            throw runtime_error("Tong tien hoan khong hop le");
        }
    }
    return totalRefund;
}

double ReportService::getActualRevenue(const string& from, const string& to)
{
    double sales = getTotalSales(from, to);
    double refund = getTotalRefund(from, to);
    double revenue = sales - refund;
    if (!isfinite(revenue))
    {
        throw runtime_error("Doanh thu thuc te khong hop le");
    }
    return revenue;
}

int ReportService::getSoldTicketCount(const string& from,
                                      const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
    int soldCount = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        Invoice* invoice = requireInvoiceForTicket(ticket);
        if (invoice->getCreatedAt() < from ||
            invoice->getCreatedAt() >= to)
        {
            continue;
        }
        if (soldCount == numeric_limits<int>::max())
        {
            throw runtime_error("So ve ban vuot gioi han");
        }
        soldCount++;
    }
    return soldCount;
}

int ReportService::getCancelledTicketCount(const string& from,
                                           const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
    int cancelledCount = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        requireInvoiceForTicket(ticket);
        if (ticket->getStatus() != TicketStatus::CANCELLED ||
            ticket->getCancelledAt() < from ||
            ticket->getCancelledAt() >= to)
        {
            continue;
        }
        if (cancelledCount == numeric_limits<int>::max())
        {
            throw runtime_error("So ve huy vuot gioi han");
        }
        cancelledCount++;
    }

    return cancelledCount;
}

double ReportService::getMovieRevenue(const string& movieId,
                                      const string& from,
                                      const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded() || !movieRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu bao cao phim");
    }
    if (movieRepo.findById(movieId) == nullptr)
    {
        throw invalid_argument("Khong tim thay phim: " + movieId);
    }
    bool hasSoldTicket = false;
    return calculateMovieRevenue(movieId, from, to, hasSoldTicket);
}

vector<Movie*> ReportService::getTopMovies(const string& from, const string& to, int limit)
{
    requireReportAccess();
    validateRange(from, to);
    if (limit <= 0)
    {
        throw invalid_argument("Gioi han top phim phai duong");
    }
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded() || !movieRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu bao cao phim");
    }
    vector<pair<Movie*, double>> ranking;
    for (int i = 0; i < movieRepo.count(); i++)
    {
        Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            throw runtime_error("Du lieu phim khong hop le");
        }
        bool hasSoldTicket = false;
        double revenue = calculateMovieRevenue(movie->getMovieId(), from, to, hasSoldTicket);
        if (hasSoldTicket)
        {
            ranking.push_back(make_pair(movie, revenue));
        }
    }
    sort(ranking.begin(), ranking.end(), compareMovieRevenue);
    vector<Movie*> result;
    for (int i = 0; i < static_cast<int>(ranking.size()) && i < limit; i++)
    {
        result.push_back(ranking[i].first);
    }
    return result;
}
