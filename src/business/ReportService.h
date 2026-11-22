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

using namespace std;

class ReportService
{
private:
    InvoiceRepository& invoiceRepo;
    TicketRepository& ticketRepo;
    MovieRepository& movieRepo;
    ShowtimeRepository& showtimeRepo;
    AuthService& authService;

    void requireReportAccess() const;
    void validateRange(const string& from, const string& to) const;
    void validateInvoice(const Invoice* invoice) const;
    Invoice* requireInvoiceForTicket(Ticket* ticket);
    double calculateMovieRevenue(const string& movieId,
                                 const string& from,
                                 const string& to,
                                 bool& hasSoldTicket);
    static bool compareMovieRevenue(const pair<Movie*, double>& first,
                                    const pair<Movie*, double>& second);

public:
    ReportService(InvoiceRepository& invoiceRepo,
                  TicketRepository& ticketRepo,
                  MovieRepository& movieRepo,
                  ShowtimeRepository& showtimeRepo,
                  AuthService& authService);

    double getTotalSales(const string& from,
                         const string& to);
    double getTotalRefund(const string& from,
                          const string& to);
    double getActualRevenue(const string& from,
                            const string& to);
    int getSoldTicketCount(const string& from,
                           const string& to);
    int getCancelledTicketCount(const string& from,
                                const string& to);
    double getMovieRevenue(const string& movieId,
                           const string& from,
                           const string& to);
    vector<Movie*> getTopMovies(const string& from,
                                const string& to,
                                int limit);
};

#endif
