#include "Ticket.h"
using namespace std;

Ticket::Ticket()
    : ticketId(""),
      invoiceId(""),
      showtimeId(""),
      seatId(""),
      ticketType(TicketType::REGULAR),
      basePrice(0.0),
      discountAmount(0.0),
      paidAmount(0.0),
      pointsEarned(0),
      status(TicketStatus::SOLD),
      cancelledAt(""),
      cancelledBy(""),
      cancelReason(""),
      refundAmount(0.0)
{}

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

void Ticket::markCancelled(const string& cancelledAt,
                           const string& cancelledBy,
                           const string& cancelReason) {
    this->status = TicketStatus::CANCELLED;
    this->cancelledAt = cancelledAt;
    this->cancelledBy = cancelledBy;
    this->cancelReason = cancelReason;
    this->refundAmount = this->paidAmount;
}
