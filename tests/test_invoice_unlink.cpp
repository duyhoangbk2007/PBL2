
#include <iostream>
#include <string>

#include "../src/models/Invoice.h"
#include "../src/models/Ticket.h"
#include "../src/models/Showtime.h"
#include "../src/models/User.h"
#include "../src/models/Member.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    if (condition)
    {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name << endl;
        failed++;
    }
}

int main()
{
    Showtime showtime(
        "SHW001", "MOV001", "ROM001",
        "2026-12-01 10:00:00",
        "2026-12-01 12:00:00",
        ShowtimeStatus::SCHEDULED
    );

    User seller(
        "USR001", "staff01", "password",
        "Nhan vien", UserRole::STAFF, true
    );

    Member member(
        "MEM001", "Nguyen Van A",
        "0901234567", 100
    );

    Invoice invoice(
        "INV001", "SHW001", "USR001",
        "MEM001", "2026-12-01 09:00:00",
        200000
    );

    Ticket ticket1(
        "TKT001", "INV001", "SHW001", "A1",
        TicketType::REGULAR,
        100000, 0, 100000, 0,
        TicketStatus::SOLD, "", "", "", 0
    );

    Ticket ticket2(
        "TKT002", "INV001", "SHW001", "A2",
        TicketType::REGULAR,
        100000, 0, 100000, 0,
        TicketStatus::SOLD, "", "", "", 0
    );

    cout << "\n===== TEST LINK =====\n";

    check(invoice.linkShowtime(&showtime),
          "Link Showtime");

    check(invoice.linkSeller(&seller),
          "Link Seller");

    check(invoice.linkMember(&member),
          "Link Member");

    check(invoice.linkTicket(&ticket1),
          "Link Ticket 1");

    check(invoice.linkTicket(&ticket2),
          "Link Ticket 2");

    check(invoice.getTicketCount() == 2,
          "Invoice co 2 Ticket");

    check(ticket1.getInvoice() == &invoice,
          "Ticket 1 tro ve Invoice");

    check(ticket2.getInvoice() == &invoice,
          "Ticket 2 tro ve Invoice");

    check(seller.getInvoiceCount() == 1,
          "Seller chua Invoice");

    check(member.getInvoiceCount() == 1,
          "Member chua Invoice");

    cout << "\n===== TEST UNLINK TICKET =====\n";

    check(invoice.unlinkTicket(&ticket1),
          "Unlink Ticket 1");

    check(ticket1.getInvoice() == nullptr,
          "Ticket 1 khong con Invoice");

    check(invoice.getTicketCount() == 1,
          "Invoice con 1 Ticket");

    check(invoice.getTicketAt(0) == &ticket2,
          "Ticket 2 van duoc lien ket");

    check(!invoice.unlinkTicket(&ticket1),
          "Khong unlink Ticket 1 hai lan");

    cout << "\n===== TEST UNLINK REFERENCES =====\n";

    invoice.unlinkReferences();

    check(invoice.getShowtime() == nullptr,
          "Showtime da unlink");

    check(invoice.getSeller() == nullptr,
          "Seller da unlink");

    check(invoice.getMember() == nullptr,
          "Member da unlink");

    check(invoice.getTicketCount() == 0,
          "Danh sach Ticket rong");

    check(ticket2.getInvoice() == nullptr,
          "Ticket 2 khong con Invoice");

    check(seller.getInvoiceCount() == 0,
          "Seller khong con Invoice");

    check(member.getInvoiceCount() == 0,
          "Member khong con Invoice");

    cout << "\n===== TEST UNLINK LAN 2 =====\n";

    invoice.unlinkReferences();

    check(invoice.getTicketCount() == 0,
          "Unlink lan 2 van an toan");

    check(invoice.getSeller() == nullptr &&
          invoice.getMember() == nullptr &&
          invoice.getShowtime() == nullptr,
          "Tat ca tham chieu van rong");

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
