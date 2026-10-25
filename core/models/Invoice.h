#ifndef PBL2_CORE_MODELS_INVOICE_H
#define PBL2_CORE_MODELS_INVOICE_H

#include <string>
#include "../data_structures/DoublyLinkedList.h"

class Showtime;
class User;
class Member;
class Ticket;

class Invoice {
private:
    std::string invoiceId;
    std::string showtimeId;
    std::string staffId;
    std::string memberId;
    std::string createdAt;
    double totalAmount;
    Showtime* showtime = nullptr;
    User* seller = nullptr;
    Member* member = nullptr;
    DoublyLinkedList<Ticket*> tickets;

public:
    Invoice();
    Invoice(const std::string& invoiceId,
            const std::string& showtimeId,
            const std::string& staffId,
            const std::string& memberId,
            const std::string& createdAt,
            double totalAmount);

    const std::string& getInvoiceId() const;
    const std::string& getShowtimeId() const;
    const std::string& getStaffId() const;
    const std::string& getMemberId() const;
    const std::string& getCreatedAt() const;
    double getTotalAmount() const;
    const Showtime* getShowtime() const;
    const User* getSeller() const;
    const Member* getMember() const;

    bool linkShowtime(Showtime* showtime);
    bool linkSeller(User* seller);
    bool linkMember(Member* member);

    bool addTicket(Ticket* ticket);
    int getTicketCount() const;
    Ticket* getTicketAt(int index);
};

#endif
