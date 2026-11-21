#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <stdexcept>
#include <chrono>
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
    if (ok) passed++; else failed++;
}
void write(const string& name, const string& contents)
{
    ofstream out(name);
    if (!out || !(out << contents)) throw runtime_error("Khong ghi file test: " + name);
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
    BookingService booking{shows, seats, tickets, invoices, members, pricing, auth};
    void load()
    {
        if (!(movies.load() && rooms.load() && seats.load() && shows.load() &&
              users.load() && members.load() && invoices.load() && tickets.load() &&
              permissions.load() && prices.load())) throw runtime_error("Load fixture that bai");
    }
};
void prepare()
{
    write("movies.txt", "MOV0001|Phim|Action|120|13|2090-01-01|2099-12-31|1\n");
    write("rooms.txt", "ROM0001|Phong|4|4|1\n");
    write("seats.txt", "ROM0001|A01|1|1|STANDARD\nROM0001|A02|1|2|STANDARD\nROM0001|A03|1|3|VIP\n");
    write("shows.txt", "SHW0001|MOV0001|ROM0001|2099-01-01T10:00:00|2099-01-01T12:00:00|SCHEDULED\n");
    write("users.txt", "USR0001|staff|123456|Staff Test|STAFF|1\nUSR0002|admin|123456|Admin Test|ADMIN|1\n");
    write("members.txt", "MEM0001|Member Test|0912345678|0\n");
    write("invoices.txt", "");
    write("tickets.txt", "");
    write("permissions.txt", "STAFF|TICKET_SELL\nSTAFF|PRICING_VIEW\nADMIN|TICKET_SELL\nADMIN|PRICING_VIEW\n");
    write("pricing.txt", "STANDARD|75000\nVIP|100000\nCOUPLE|160000\nSTUDENT_DISCOUNT|10\nMEMBER_DISCOUNT|0\n");
}
int main()
{
    fs::path old = fs::current_path();
    fs::path dir = fs::temp_directory_path() /
        ("pbl2_booking_service_test_" + to_string(chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(dir);
    fs::current_path(dir);
    try
    {
        prepare();
        {
            Fixture f;
            f.load();
            check(f.auth.login("staff", "123456"), "login Staff");
            BookingSummary summary;
            vector<BookingItem> items{{"A01", TicketType::REGULAR}, {"A02", TicketType::REGULAR}};
            check(f.booking.getAvailableSeats("SHW0001").size() == 3, "three available seats");
            check(f.booking.calculateBooking("SHW0001", items, "MEM0001", summary), "calculate two tickets");
            check(summary.ticketPrices.size() == 2 && summary.totalAmount == 150000 && summary.earnedPoints == 14,
                  "points summed per ticket, not invoice");
            string id = "stale";
            check(!f.booking.createBooking("SHW0001", items, "MEM0001", false, true, id) && id.empty(), "age confirmation required");
            check(!f.booking.createBooking("SHW0001", items, "MEM0001", true, false, id), "payment confirmation required");
            check(f.invoices.count() == 0 && f.tickets.count() == 0, "no records on rejection");
            check(!f.booking.calculateBooking("SHW0001", {{"A01", TicketType::REGULAR}, {"A01", TicketType::STUDENT}}, "", summary) && summary.ticketPrices.empty(), "duplicate selection clears summary");
            check(!f.booking.calculateBooking("SHW0001", {{"A99", TicketType::REGULAR}}, "", summary), "seat belongs to room");
            check(!f.booking.calculateBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "MEM9999", summary), "unknown member");
            check(f.booking.calculateBooking("SHW0001", {{"A03", TicketType::STUDENT}}, "MEM0001", summary) &&
                  summary.totalAmount == 90000 && summary.earnedPoints == 9, "VIP student discount and points");
            check(!f.booking.calculateBooking("SHW0001", {{"A03", static_cast<TicketType>(99)}}, "", summary) &&
                  summary.ticketPrices.empty(), "invalid ticket type");
            check(f.booking.createBooking("SHW0001", items, "MEM0001", true, true, id) && id == "INV0001", "sell two tickets");
            check(f.invoices.count() == 1 && f.tickets.count() == 2 && f.members.findById("MEM0001")->getPoints() == 14, "invoice, tickets, member points");
            Invoice* invoice = f.invoices.findById(id);
            check(invoice && invoice->getTicketCount() == 2 && invoice->getSeller() && invoice->getMember(), "invoice links");
            check(f.tickets.findById("TKT0001") && f.tickets.findById("TKT0001")->getPointsEarned() == 7 &&
                  f.tickets.findById("TKT0002")->getPointsEarned() == 7, "ticket points and IDs");
            check(f.booking.getAvailableSeats("SHW0001").size() == 1, "sold seats excluded");
            check(!f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "", true, true, id), "cannot resell sold seat");
            check(f.invoices.count() == 1 && f.tickets.count() == 2, "resell rejection leaves counts");
            f.auth.logout();
            bool denied = false;
            try { f.booking.createBooking("SHW0001", {{"A03", TicketType::REGULAR}}, "", true, true, id); }
            catch (const exception&) { denied = true; }
            check(denied, "login required");
        }
        {
            Fixture f;
            f.load();
            check(f.invoices.count() == 1 && f.tickets.count() == 2 && f.members.findById("MEM0001")->getPoints() == 14, "reload saved transaction");
            check(!f.tickets.rollbackPending("TKT0001") && !f.invoices.rollbackPending("INV0001"), "saved history cannot rollback");
            check(f.auth.login("admin", "123456"), "login Admin");
            string id;
            check(f.booking.createBooking("SHW0001", {{"A03", TicketType::REGULAR}}, "", true, true, id) && id == "INV0002", "Admin can sell remaining seat");
        }
        prepare();
        {
            Fixture f;
            f.load();
            Invoice* invoice = new Invoice("INV0001", "SHW0001", "USR0001", "MEM0001",
                                           "2098-01-01T09:00:00", 75000);
            check(f.invoices.add(invoice), "add pending invoice");
            Ticket* ticket = new Ticket("TKT0001", "INV0001", "SHW0001", "A01",
                                        TicketType::REGULAR, 75000, 0, 75000, 7,
                                        TicketStatus::SOLD, "", "", "", 0);
            check(f.tickets.add(ticket), "add pending ticket");
            check(!f.invoices.rollbackPending("INV0001"), "cannot rollback invoice with ticket");
            check(f.tickets.rollbackPending("TKT0001"), "rollback pending ticket");
            check(invoice->getTicketCount() == 0 && f.tickets.count() == 0, "ticket links removed");
            check(f.invoices.rollbackPending("INV0001"), "rollback pending invoice");
            check(f.invoices.count() == 0, "invoice removed");
        }
        prepare();
        {
            Fixture f;
            f.load();
            f.auth.login("staff", "123456");
            fs::remove("invoices.txt");
            fs::create_directory("invoices.txt");
            string id;
            bool storageError = false;
            try { f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "MEM0001", true, true, id); }
            catch (const exception&) { storageError = true; }
            check(storageError && id.empty(), "invoice save failure throws");
            check(f.invoices.count() == 1 && f.tickets.count() == 1, "RAM retained after uncertain file write");
            bool blocked = false;
            try { f.booking.createBooking("SHW0001", {{"A02", TicketType::REGULAR}}, "", true, true, id); }
            catch (const exception&) { blocked = true; }
            check(blocked, "further sale blocked after storage failure");
            fs::remove("invoices.txt");
        }
        prepare();
        {
            Fixture f;
            f.load();
            f.auth.login("staff", "123456");
            fs::remove("tickets.txt");
            fs::create_directory("tickets.txt");
            string id;
            bool storageError = false;
            try { f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "", true, true, id); }
            catch (const exception&) { storageError = true; }
            check(storageError && id.empty(), "ticket save failure throws");
            check(f.invoices.count() == 1 && f.tickets.count() == 1, "RAM retained after ticket save failure");
            fs::remove("tickets.txt");
        }
        prepare();
        {
            Fixture f;
            f.load();
            f.auth.login("staff", "123456");
            fs::remove("members.txt");
            fs::create_directory("members.txt");
            string id;
            bool storageError = false;
            try { f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}}, "MEM0001", true, true, id); }
            catch (const exception&) { storageError = true; }
            check(storageError && id.empty(), "member save failure throws");
            check(f.members.findById("MEM0001")->getPoints() == 7 && f.tickets.count() == 1,
                  "RAM retained after member save failure");
            fs::remove("members.txt");
        }
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
