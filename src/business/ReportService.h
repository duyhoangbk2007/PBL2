#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include <string>
#include <utility>
#include <vector>

#include "../data_access/InvoiceRepository.h"
#include "../data_access/TicketRepository.h"
#include "../data_access/MovieRepository.h"
#include "../data_access/ShowtimeRepository.h"
#include "AuthService.h"


class ReportService
{
private:
    InvoiceRepository& invoiceRepo;
    TicketRepository& ticketRepo;
    MovieRepository& movieRepo;
    ShowtimeRepository& showtimeRepo;
    AuthService& authService;

    void requireReportAccess() const;
    void validateRange(const std::string& from, const std::string& to) const;
    void validateInvoice(const Invoice* invoice) const;
    Invoice* requireInvoiceForTicket(Ticket* ticket);
    double calculateMovieRevenue(const std::string& movieId,
                                 const std::string& from,
                                 const std::string& to,
                                 bool& hasSoldTicket);
    static bool compareMovieRevenue(const std::pair<Movie*, double>& first,
                                    const std::pair<Movie*, double>& second);

public:
    ReportService(InvoiceRepository& invoiceRepo,
                  TicketRepository& ticketRepo,
                  MovieRepository& movieRepo,
                  ShowtimeRepository& showtimeRepo,
                  AuthService& authService);

    double getTotalSales(const std::string& from,
                         const std::string& to);
    double getTotalRefund(const std::string& from,
                          const std::string& to);
    double getActualRevenue(const std::string& from,
                            const std::string& to);
    int getSoldTicketCount(const std::string& from,
                           const std::string& to);
    int getCancelledTicketCount(const std::string& from,
                                const std::string& to);
    double getMovieRevenue(const std::string& movieId,
                           const std::string& from,
                           const std::string& to);
    std::vector<Movie*> getTopMovies(const std::string& from,
                                const std::string& to,
                                int limit);
};

#endif
