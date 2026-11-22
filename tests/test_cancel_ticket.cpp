
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <vector>

#include "../src/business/BookingService.h"
#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/data_access/PricingRepository.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool ok, const string& label)
{
    cout << (ok ? "[PASS] " : "[FAIL] ") << label << '\n';

    if (ok)
    {
        passed++;
    }
    else
    {
        failed++;
    }
}

void write(const string& name, const string& contents)
{
    ofstream out(name);

    if (!out || !(out << contents))
    {
        throw runtime_error("Khong ghi file test: " + name);
    }
}

struct Fixture
{
    MovieRepository movies{"movies.txt"};
    RoomRepository rooms{"rooms.txt"};
    SeatRepository seats{"seats.txt", rooms};
    ShowtimeRepository shows{"shows.txt", movies, rooms};
    UserRepository users{"users.txt"};
    MemberRepository members{"members.txt"};
    InvoiceRepository invoices{"invoices.txt", shows, users, members};
    TicketRepository tickets{"tickets.txt", shows, seats, invoices};
    RolePermissionRepository permissions{"permissions.txt"};
    PricingRepository prices{"pricing.txt"};

    AuthService auth{users, permissions};
    PricingService pricing{prices, auth};

    BookingService booking{
        shows, seats, tickets, invoices,
        members, pricing, auth
    };

    void load()
    {
        if (!(movies.load() &&
              rooms.load() &&
              seats.load() &&
              shows.load() &&
              users.load() &&
              members.load() &&
              invoices.load() &&
              tickets.load() &&
              permissions.load() &&
              prices.load()))
        {
            throw runtime_error("Load fixture that bai");
        }
    }
};

void prepare()
{
    write("movies.txt",
          "MOV0001|Phim|Action|120|13|2090-01-01|2099-12-31|1\n");

    write("rooms.txt",
          "ROM0001|Phong|4|4|1\n");

    write("seats.txt",
          "ROM0001|A01|1|1|STANDARD\n"
          "ROM0001|A02|1|2|STANDARD\n"
          "ROM0001|A03|1|3|VIP\n");

    write("shows.txt",
          "SHW0001|MOV0001|ROM0001|2099-01-01T10:00:00|2099-01-01T12:00:00|SCHEDULED\n");

    write("users.txt",
          "USR0001|staff|123456|Staff Test|STAFF|1\n"
          "USR0002|admin|123456|Admin Test|ADMIN|1\n");

    write("members.txt",
          "MEM0001|Member Test|0912345678|0\n");

    write("invoices.txt", "");
    write("tickets.txt", "");

    write("permissions.txt",
          "STAFF|TICKET_SELL\n"
          "STAFF|PRICING_VIEW\n"
          "ADMIN|TICKET_SELL\n"
          "ADMIN|PRICING_VIEW\n"
          "ADMIN|TICKET_CANCEL\n");

    write("pricing.txt",
          "STANDARD|75000\n"
          "VIP|100000\n"
          "COUPLE|160000\n"
          "STUDENT_DISCOUNT|10\n"
          "MEMBER_DISCOUNT|0\n");
}

static void sell(Fixture& f, const vector<BookingItem>& items, const string& memberId)
{
    if (!f.auth.login("staff", "123456")) throw runtime_error("Staff login failed");
    string invoiceId;
    if (!f.booking.createBooking("SHW0001", items, memberId, true, true, invoiceId))
        throw runtime_error("Prepare sale failed");
    f.auth.logout();
}

static bool denied(Fixture& f)
{
    try { f.booking.cancelTicket("TKT0001", "Khach huy", true); }
    catch (const exception&) { return true; }
    return false;
}

void testValidCancellation()
{
    prepare();
    Fixture f;
    f.load();
    sell(f, {{"A01", TicketType::REGULAR}, {"A02", TicketType::REGULAR}}, "MEM0001");
    check(f.auth.login("admin", "123456"), "Admin login");
    check(f.booking.cancelTicket("TKT0001", "Khach huy", true), "Cancel first ticket");
    Ticket* first = f.tickets.findById("TKT0001");
    Ticket* second = f.tickets.findById("TKT0002");
    Invoice* invoice = f.invoices.findById("INV0001");
    check(first && first->getStatus() == TicketStatus::CANCELLED &&
          first->getCancelledBy() == "USR0002" && first->getCancelReason() == "Khach huy" &&
          first->getRefundAmount() == first->getPaidAmount(), "Cancellation metadata and refund");
    check(f.members.findById("MEM0001")->getPoints() == 7 && first->getPointsEarned() == 7,
          "Recover only cancelled ticket points");
    check(second && second->getStatus() == TicketStatus::SOLD &&
          invoice && invoice->getTicketCount() == 2 && invoice->getTotalAmount() == 150000,
          "Other ticket and original invoice remain");
    check(!f.tickets.isSeatSold("SHW0001", "A01") &&
          f.tickets.isSeatSold("SHW0001", "A02") &&
          f.booking.getAvailableSeats("SHW0001").size() == 2, "Seat returned without changing Seat");
    check(!f.booking.cancelTicket("TKT0001", "Again", true), "Cannot cancel twice");
    check(!f.booking.cancelTicket("TKT9999", "Unknown", true), "Unknown ticket");
    string id;
    check(f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "", true, true, id) &&
          id == "INV0002" && f.tickets.findById("TKT0003") != nullptr,
          "Resell returned seat with new ticket ID");
    check(first->getStatus() == TicketStatus::CANCELLED, "Cancelled ticket history remains");
}

void testGuards()
{
    prepare();
    Fixture f;
    f.load();
    sell(f, {{"A01", TicketType::REGULAR}}, "MEM0001");
    check(denied(f), "Login required");
    f.auth.login("staff", "123456");
    check(denied(f), "Staff denied despite permission table");
    f.auth.logout();
    f.auth.login("admin", "123456");
    Ticket* ticket = f.tickets.findById("TKT0001");
    Member* member = f.members.findById("MEM0001");
    check(!f.booking.cancelTicket("TKT0001", "Khach huy", false), "Refund confirmation required");
    check(!f.booking.cancelTicket("TKT0001", "   ", true), "Reason required");
    check(!f.booking.cancelTicket("TKT0001", "Bad|reason", true), "Reason cannot contain separator");
    check(ticket->getStatus() == TicketStatus::SOLD && member->getPoints() == 7,
          "Rejected cancellation leaves RAM unchanged");
    member->setPoints(6);
    check(!f.booking.cancelTicket("TKT0001", "Khach huy", true), "Insufficient points rejected");
    check(ticket->getStatus() == TicketStatus::SOLD && member->getPoints() == 6,
          "Insufficient points leaves ticket unchanged");
}

void testMissingPermission()
{
    prepare();
    write("permissions.txt", "STAFF|TICKET_SELL\nSTAFF|PRICING_VIEW\nADMIN|TICKET_SELL\nADMIN|PRICING_VIEW\n");
    Fixture f;
    f.load();
    sell(f, {{"A01", TicketType::REGULAR}}, "");
    f.auth.login("admin", "123456");
    check(denied(f), "Admin without TICKET_CANCEL denied");
}

void testTimeAndLinks()
{
    prepare();
    {
        Fixture f;
        f.load();
        sell(f, {{"A01", TicketType::REGULAR}}, "");
        f.auth.login("admin", "123456");
        Showtime* show = f.shows.findById("SHW0001");
        show->setStartAt("2020-01-01T10:00:00");
        check(!f.booking.cancelTicket("TKT0001", "Khach huy", true),
              "Started showtime cannot be cancelled");
        check(f.tickets.findById("TKT0001")->getStatus() == TicketStatus::SOLD,
              "Started showtime leaves ticket unchanged");
        show->setStartAt("2099-01-01T10:00:00");
        f.tickets.findById("TKT0001")->unlinkReferences();
        check(!f.booking.cancelTicket("TKT0001", "Khach huy", true),
              "Missing Ticket links rejected");
    }
    prepare();
    write("invoices.txt", "INV0001|SHW0001|USR0001||2098-01-01T09:00:00|75000\n");
    write("tickets.txt", "TKT0001|INV0001|SHW0001|A01|REGULAR|75000|0|75000|0|SOLD||||0\n");
    {
        Fixture f;
        f.load();
        f.auth.login("admin", "123456");
        check(!f.booking.cancelTicket("TKT0001", "Khach huy", true),
              "Cancellation before invoice creation rejected");
    }
}

void testReloadAndNoMember()
{
    prepare();
    {
        Fixture f;
        f.load();
        sell(f, {{"A01", TicketType::REGULAR}}, "");
        f.auth.login("admin", "123456");
        check(f.booking.cancelTicket("TKT0001", "Khach doi lich", true), "Cancel without member");
    }
    {
        Fixture f;
        f.load();
        Ticket* ticket = f.tickets.findById("TKT0001");
        check(ticket && ticket->getStatus() == TicketStatus::CANCELLED &&
              ticket->getCancelReason() == "Khach doi lich" &&
              ticket->getRefundAmount() == ticket->getPaidAmount(), "Reload cancelled ticket");
        check(!f.tickets.isSeatSold("SHW0001", "A01"), "Reloaded seat available");
    }
}

void testSaveFailures()
{
    prepare();
    {
        Fixture f;
        f.load();
        sell(f, {{"A01", TicketType::REGULAR}}, "");
        f.auth.login("admin", "123456");
        fs::remove("tickets.txt");
        fs::create_directory("tickets.txt");
        bool threw = false;
        try { f.booking.cancelTicket("TKT0001", "Khach huy", true); }
        catch (const exception&) { threw = true; }
        check(threw, "Ticket save failure throws");
        check(f.tickets.findById("TKT0001")->getStatus() == TicketStatus::CANCELLED,
              "RAM retained after Ticket write failure");
        bool blocked = false;
        try { f.booking.cancelTicket("TKT0001", "Again", true); }
        catch (const exception&) { blocked = true; }
        check(blocked, "Further cancellation blocked after write failure");
        string id;
        blocked = false;
        try { f.booking.createBooking("SHW0001", {{"A02", TicketType::REGULAR}}, "", true, true, id); }
        catch (const exception&) { blocked = true; }
        check(blocked, "Further sale blocked after write failure");
        fs::remove("tickets.txt");
    }
    prepare();
    {
        Fixture f;
        f.load();
        sell(f, {{"A01", TicketType::REGULAR}}, "MEM0001");
        f.auth.login("admin", "123456");
        fs::remove("members.txt");
        fs::create_directory("members.txt");
        bool threw = false;
        try { f.booking.cancelTicket("TKT0001", "Khach huy", true); }
        catch (const exception&) { threw = true; }
        check(threw, "Member save failure throws");
        check(f.tickets.findById("TKT0001")->getStatus() == TicketStatus::CANCELLED &&
              f.members.findById("MEM0001")->getPoints() == 0,
              "RAM retained after Member write failure");
        fs::remove("members.txt");
    }
}

int main()
{
    fs::path old = fs::current_path();
    fs::path dir = fs::temp_directory_path() /
        ("pbl2_cancel_ticket_test_" +
         to_string(chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(dir);
    fs::current_path(dir);
    try
    {
        testValidCancellation();
        testGuards();
        testMissingPermission();
        testTimeAndLinks();
        testReloadAndNoMember();
        testSaveFailures();
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] " << e.what() << '\n';
        failed++;
    }
    fs::current_path(old);
    fs::remove_all(dir);
    cout << "Passed: " << passed << " Failed: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}
