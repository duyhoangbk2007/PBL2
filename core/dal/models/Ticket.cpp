#include "Ticket.h"
#include "Seat.h"
#include "Showtime.h"
using namespace std;

Ticket::Ticket(const string& ticketId,
               const string& invoiceId,
               const string& showtimeId,
               const string& seatId,
               TicketType ticketType,
               double basePrice,
               double discountAmount,
               double paidAmount,
               int pointsEarned,
               TicketStatus status,
               const string& cancelledAt,
               const string& cancelledBy,
               const string& cancelReason,
               double refundAmount)
    : ticketId(ticketId),
      invoiceId(invoiceId),
      showtimeId(showtimeId),
      seatId(seatId),
      ticketType(ticketType),
      basePrice(basePrice),
      discountAmount(discountAmount),
      paidAmount(paidAmount),
      pointsEarned(pointsEarned),
      status(status),
      cancelledAt(cancelledAt),
      cancelledBy(cancelledBy),
      cancelReason(cancelReason),
      refundAmount(refundAmount)
{}

const string& Ticket::getTicketId() const {
    return this->ticketId;
}

const string& Ticket::getInvoiceId() const {
    return this->invoiceId;
}

const string& Ticket::getShowtimeId() const {
    return this->showtimeId;
}

const string& Ticket::getSeatId() const {
    return this->seatId;
}

TicketType Ticket::getTicketType() const {
    return this->ticketType;
}

double Ticket::getBasePrice() const {
    return this->basePrice;
}

double Ticket::getDiscountAmount() const {
    return this->discountAmount;
}

double Ticket::getPaidAmount() const {
    return this->paidAmount;
}

int Ticket::getPointsEarned() const {
    return this->pointsEarned;
}

TicketStatus Ticket::getStatus() const {
    return this->status;
}

const string& Ticket::getCancelledAt() const {
    return this->cancelledAt;
}

const string& Ticket::getCancelledBy() const {
    return this->cancelledBy;
}

const string& Ticket::getCancelReason() const {
    return this->cancelReason;
}

double Ticket::getRefundAmount() const {
    return this->refundAmount;
}

const Showtime* Ticket::getShowtime() const {
    return this->showtime;
}

const Seat* Ticket::getSeat() const {
    return this->seat;
}

bool Ticket::linkShowtime(Showtime* showtime) {
    if (showtime == nullptr || showtime->getShowtimeId() != this->showtimeId ||
        (this->seat != nullptr && this->seat->getRoomId() != showtime->getRoomId())) {
        return false;
    }

    this->showtime = showtime;
    return true;
}

bool Ticket::linkSeat(Seat* seat) {
    if (seat == nullptr || this->showtime == nullptr ||
        seat->getSeatId() != this->seatId ||
        seat->getRoomId() != this->showtime->getRoomId()) {
        return false;
    }

    this->seat = seat;
    return true;
}

void Ticket::markCancelled(const string& cancelledAt,
                           const string& cancelledBy,
                           const string& cancelReason) {
    this->status = TicketStatus::CANCELLED;
    this->cancelledAt = cancelledAt;
    this->cancelledBy = cancelledBy;
    this->cancelReason = cancelReason;
    this->refundAmount = this->paidAmount;
}
