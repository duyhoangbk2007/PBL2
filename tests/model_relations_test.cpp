#include "../core/dal/models/Invoice.h"
#include "../core/dal/models/Member.h"
#include "../core/dal/models/Ticket.h"
#include "../core/dal/models/User.h"

#include <cassert>
#include <type_traits>

static_assert(!std::is_copy_constructible<User>::value, "User must not be copied");
static_assert(!std::is_copy_assignable<User>::value, "User must not be copy-assigned");
static_assert(!std::is_move_constructible<User>::value, "User must not be moved");
static_assert(!std::is_move_assignable<User>::value, "User must not be move-assigned");
static_assert(!std::is_copy_constructible<Member>::value, "Member must not be copied");
static_assert(!std::is_copy_assignable<Member>::value, "Member must not be copy-assigned");
static_assert(!std::is_move_constructible<Member>::value, "Member must not be moved");
static_assert(!std::is_move_assignable<Member>::value, "Member must not be move-assigned");

static Ticket makeTicket(const char* ticketId,
                         const char* invoiceId,
                         const char* showtimeId,
                         int points,
                         double paidAmount) {
    return Ticket(ticketId, invoiceId, showtimeId, "SEAT-1", TicketType::REGULAR,
                  paidAmount + 25.0, 25.0, paidAmount, points, TicketStatus::SOLD,
                  "", "", "", 0.0);
}

int main() {
    User seller("U1", "seller", "pw", "Seller", UserRole::STAFF, false);
    User wrongSeller("U2", "other", "pw", "Other", UserRole::STAFF, true);
    Member member("M1", "Member", "0900", 50);
    Member wrongMember("M2", "Other Member", "0901", 20);
    Member emptyIdMember("", "Guest-like", "", 0);

    Invoice invoice("I1", "S1", "U1", "M1", "2026-11-03", 150.0);
    Invoice secondInvoice("I2", "S1", "U1", "M1", "2026-11-03", 20.0);
    Invoice guestInvoice("I3", "S1", "U1", "", "2026-11-03", 0.0);
    Invoice duplicateInvoice("I1", "S1", "U1", "M1", "2026-11-03", 150.0);

    assert(seller.getUserId() == "U1");
    assert(seller.getRole() == UserRole::STAFF);
    assert(!seller.isActive());
    assert(member.getMemberId() == "M1");
    assert(member.getFullName() == "Member");
    assert(member.getPhone() == "0900");
    assert(member.getPoints() == 50);
    assert(invoice.getInvoiceId() == "I1");
    assert(invoice.getShowtimeId() == "S1");
    assert(invoice.getStaffId() == "U1");
    assert(invoice.getMemberId() == "M1");
    assert(invoice.getCreatedAt() == "2026-11-03");
    assert(invoice.getTotalAmount() == 150.0);

    assert(!invoice.linkSeller(nullptr));
    assert(!invoice.linkSeller(&wrongSeller));
    assert(invoice.linkSeller(&seller));
    assert(invoice.linkSeller(&seller));
    assert(invoice.getSeller() == &seller);
    assert(seller.getInvoiceCount() == 1);
    assert(seller.getInvoiceAt(0) == &invoice);
    assert(!invoice.linkSeller(&wrongSeller));
    assert(!duplicateInvoice.linkSeller(&seller));
    assert(seller.getInvoiceCount() == 1);

    assert(!invoice.linkMember(nullptr));
    assert(!invoice.linkMember(&wrongMember));
    assert(invoice.linkMember(&member));
    assert(invoice.linkMember(&member));
    assert(invoice.getMember() == &member);
    assert(member.getInvoiceCount() == 1);
    assert(member.getInvoiceAt(0) == &invoice);
    assert(!invoice.linkMember(&wrongMember));
    assert(!duplicateInvoice.linkMember(&member));
    assert(secondInvoice.linkMember(&member));
    assert(member.getInvoiceCount() == 2);
    assert(member.getInvoiceAt(1) == &secondInvoice);
    assert(member.getInvoiceAt(-1) == nullptr);
    assert(member.getInvoiceAt(2) == nullptr);
    assert(guestInvoice.linkMember(nullptr));
    assert(guestInvoice.getMember() == nullptr);
    assert(!guestInvoice.linkMember(&member));
    assert(!guestInvoice.linkMember(&emptyIdMember));
    assert(member.getInvoiceCount() == 2);
    assert(emptyIdMember.getInvoiceCount() == 0);

    Ticket ticket = makeTicket("T1", "I1", "S1", 12, 75.0);
    Ticket secondTicket = makeTicket("T2", "I1", "S1", 8, 75.0);
    Ticket duplicateTicket = makeTicket("T1", "I1", "S1", 12, 75.0);
    Ticket wrongInvoiceTicket = makeTicket("T3", "I2", "S1", 3, 10.0);
    Ticket wrongShowtimeTicket = makeTicket("T4", "I1", "S2", 4, 10.0);

    assert(ticket.getTicketId() == "T1");
    assert(ticket.getInvoiceId() == "I1");
    assert(ticket.getShowtimeId() == "S1");
    assert(ticket.getPointsEarned() == 12);
    assert(ticket.getStatus() == TicketStatus::SOLD);
    assert(!invoice.linkTicket(nullptr));
    assert(!invoice.linkTicket(&wrongInvoiceTicket));
    assert(!invoice.linkTicket(&wrongShowtimeTicket));
    assert(invoice.linkTicket(&ticket));
    assert(invoice.linkTicket(&ticket));
    assert(ticket.getInvoice() == &invoice);
    assert(invoice.getTicketCount() == 1);
    assert(invoice.getTicketAt(0) == &ticket);
    assert(invoice.getTicketAt(-1) == nullptr);
    assert(invoice.getTicketAt(1) == nullptr);
    assert(!invoice.linkTicket(&duplicateTicket));
    assert(!duplicateInvoice.linkTicket(&ticket));
    assert(invoice.linkTicket(&secondTicket));
    assert(invoice.getTicketCount() == 2);
    assert(secondInvoice.linkTicket(&wrongInvoiceTicket));
    assert(wrongInvoiceTicket.getInvoice() == &secondInvoice);

    const double originalTotal = invoice.getTotalAmount();
    const int originalPoints = ticket.getPointsEarned();
    ticket.markCancelled("2026-11-03T16:00", "U1", "customer request");
    assert(ticket.getStatus() == TicketStatus::CANCELLED);
    assert(ticket.getCancelledAt() == "2026-11-03T16:00");
    assert(ticket.getCancelledBy() == "U1");
    assert(ticket.getCancelReason() == "customer request");
    assert(ticket.getRefundAmount() == ticket.getPaidAmount());
    assert(ticket.getPointsEarned() == originalPoints);
    assert(invoice.getTotalAmount() == originalTotal);
}
