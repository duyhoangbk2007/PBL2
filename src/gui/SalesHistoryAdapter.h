#pragma once
#include <QDateTime>
#include <QList>
#include <QString>

class AuthService;

// Presentation data. Amounts, rankings and eligibility come from the source.
namespace SalesHistory {
enum class State { Available, NotImplemented, Error, Preview };
struct Status {
    State state = State::NotImplemented;
    QString message;
};
struct Period {
    QDateTime from, until;
    bool valid() const;
    bool contains(const QDateTime& time) const;
};
struct InvoiceFilter {
    Period period;
    QString code, staff, movie, show;
};
struct TicketLine {
    QString id, seat, seatType, ticketType, status;
    double base, discount, paid;
    QString cancelledAt, cancelledBy, reason;
    double refunded;
    bool canCancel;
    double proposedRefund;
};
struct Invoice {
    QString id, staff, movie, show, room, member, status;
    QDateTime created, starts;
    double original, refunded, remaining;
    QList<TicketLine> tickets;
};
struct InvoiceResult {
    Status status;
    QList<Invoice> invoices;
};
struct ChartEntry {
    QString label;
    double amount;
    int tickets = 0;
};
struct Report {
    Status status;
    QList<ChartEntry> daily, movies;
    double sold = 0, refunded = 0, net = 0;
    int issued = 0, cancelled = 0;
};
InvoiceResult history(const AuthService& auth, const InvoiceFilter& filter, bool preview);
InvoiceResult cancellationLookup(const AuthService& auth, const QString& code, bool preview);
Report report(const AuthService& auth, const Period& period, bool preview);
}
