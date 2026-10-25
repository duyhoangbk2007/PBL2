#include "Invoice.h"
#include "Member.h"
#include "Showtime.h"
#include "Ticket.h"
#include "User.h"
using namespace std;

Invoice::Invoice()
    : invoiceId(""),
      showtimeId(""),
      staffId(""),
      memberId(""),
      createdAt(""),
      totalAmount(0.0)
{}

Invoice::Invoice(const string& invoiceId,
                 const string& showtimeId,
                 const string& staffId,
                 const string& memberId,
                 const string& createdAt,
                 double totalAmount)
    : invoiceId(invoiceId),
      showtimeId(showtimeId),
      staffId(staffId),
      memberId(memberId),
      createdAt(createdAt),
      totalAmount(totalAmount)
{}

const string& Invoice::getInvoiceId() const {
    return this->invoiceId;
}

const string& Invoice::getShowtimeId() const {
    return this->showtimeId;
}

const string& Invoice::getStaffId() const {
    return this->staffId;
}

const string& Invoice::getMemberId() const {
    return this->memberId;
}

const string& Invoice::getCreatedAt() const {
    return this->createdAt;
}

double Invoice::getTotalAmount() const {
    return this->totalAmount;
}

const Showtime* Invoice::getShowtime() const {
    return this->showtime;
}

const User* Invoice::getSeller() const {
    return this->seller;
}

const Member* Invoice::getMember() const {
    return this->member;
}

bool Invoice::linkShowtime(Showtime* showtime) {
    if (showtime == nullptr || showtime->getShowtimeId() != this->showtimeId) {
        return false;
    }

    this->showtime = showtime;
    return true;
}

bool Invoice::linkSeller(User* seller) {
    if (seller == nullptr || seller->getUserId() != this->staffId) {
        return false;
    }

    this->seller = seller;
    return true;
}

bool Invoice::linkMember(Member* member) {
    if (member == nullptr) {
        if (!this->memberId.empty()) {
            return false;
        }

        this->member = nullptr;
        return true;
    }

    if (member->getMemberId() != this->memberId) {
        return false;
    }

    this->member = member;
    return true;
}

bool Invoice::addTicket(Ticket* ticket) {
    if (ticket == nullptr || ticket->getInvoiceId() != this->invoiceId ||
        ticket->getShowtimeId() != this->showtimeId) {
        return false;
    }

    for (int index = 0; index < this->tickets.nodeCount(); ++index) {
        Ticket* current = this->tickets.getAt(index)->data;
        if (current->getTicketId() == ticket->getTicketId()) {
            return false;
        }
    }

    this->tickets.pushBack(ticket);
    return true;
}

int Invoice::getTicketCount() const {
    return this->tickets.nodeCount();
}

Ticket* Invoice::getTicketAt(int index) {
    ListNode<Ticket*>* node = this->tickets.getAt(index);
    return node == nullptr ? nullptr : node->data;
}
