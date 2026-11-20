#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <exception>

#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/SeatRepository.h"
#include "../src/data_access/ShowtimeRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/MemberRepository.h"
#include "../src/data_access/InvoiceRepository.h"
#include "../src/data_access/TicketRepository.h"

using namespace std;

static int passed = 0;
static int failed = 0;

static const string MOVIES = "test_it_movies_temp.txt";
static const string ROOMS = "test_it_rooms_temp.txt";
static const string SEATS = "test_it_seats_temp.txt";
static const string SHOWS = "test_it_showtimes_temp.txt";
static const string USERS = "test_it_users_temp.txt";
static const string MEMBERS = "test_it_members_temp.txt";
static const string INVOICES = "test_it_invoices_temp.txt";
static const string TICKETS = "test_it_tickets_temp.txt";

void check(const string& name, bool condition)
{
    if (condition)
    {
        cout << "[PASS] " << name << '\n';
        passed++;
    }
    else
    {
        cout << "[FAIL] " << name << '\n';
        failed++;
    }
}

void writeFile(const string& path, const string& content)
{
    ofstream out(path, ios::trunc);
    if (!out.is_open())
    {
        throw runtime_error("Khong tao duoc file: " + path);
    }
    out << content;
}

void cleanup()
{
    remove(MOVIES.c_str());
    remove(ROOMS.c_str());
    remove(SEATS.c_str());
    remove(SHOWS.c_str());
    remove(USERS.c_str());
    remove(MEMBERS.c_str());
    remove(INVOICES.c_str());
    remove(TICKETS.c_str());
}

const string VALID_INVOICE =
    "INV0001|SHW0001|USR0001|MEM0001|2027-01-10T09:00:00|70000\n";
const string VALID_TICKET =
    "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\n";

void prepare(const string& invoiceLines = "", const string& ticketLines = "")
{
    writeFile(MOVIES, "MOV0001|Phim 1|Action|120|13|2026-01-01|2028-12-31|1\n");
    writeFile(ROOMS, "ROM0001|Phong 1|8|12|1\n");
    writeFile(SEATS, "ROM0001|A01|1|1|STANDARD\nROM0001|A02|1|2|STANDARD\nROM0001|A03|1|3|STANDARD\n");
    writeFile(SHOWS, "SHW0001|MOV0001|ROM0001|2027-01-10T10:00:00|2027-01-10T12:00:00|SCHEDULED\n");
    writeFile(USERS, "USR0001|staff01|pass123|Nhan vien A|STAFF|1\nUSR0002|admin01|pass123|Quan tri|ADMIN|1\n");
    writeFile(MEMBERS, "MEM0001|Nguyen Van A|0901234567|0\n");
    writeFile(INVOICES, invoiceLines);
    writeFile(TICKETS, ticketLines);
}

// Thu tu khai bao dam bao destructor chay: Ticket -> Invoice -> cac repo phu thuoc.
struct Fixture
{
    MovieRepository movies;
    RoomRepository rooms;
    SeatRepository seats;
    ShowtimeRepository shows;
    UserRepository users;
    MemberRepository members;
    InvoiceRepository invoices;
    TicketRepository tickets;

    Fixture()
        : movies(MOVIES),
          rooms(ROOMS),
          seats(SEATS, rooms),
          shows(SHOWS, movies, rooms),
          users(USERS),
          members(MEMBERS),
          invoices(INVOICES, shows, users, members),
          tickets(TICKETS, shows, seats, invoices)
    {
    }

    bool loadDependencies()
    {
        return movies.load() && rooms.load() && seats.load() &&
               shows.load() && users.load() && members.load();
    }
};

bool tryLoadInvoice(InvoiceRepository& repo)
{
    try { return repo.load(); }
    catch (const exception& e)
    {
        cout << "  (Invoice exception: " << e.what() << ")\n";
        return false;
    }
}

bool tryLoadTicket(TicketRepository& repo)
{
    try { return repo.load(); }
    catch (const exception& e)
    {
        cout << "  (Ticket exception: " << e.what() << ")\n";
        return false;
    }
}

Ticket* makeTicket(const string& id, const string& seat,
                   const string& invoiceId = "INV0001",
                   TicketStatus status = TicketStatus::SOLD)
{
    if (status == TicketStatus::CANCELLED)
    {
        return new Ticket(id, invoiceId, "SHW0001", seat,
                          TicketType::REGULAR, 70000, 0, 70000, 7,
                          status, "2027-01-10T09:30:00", "USR0002",
                          "Khach huy", 70000);
    }
    return new Ticket(id, invoiceId, "SHW0001", seat,
                      TicketType::REGULAR, 70000, 0, 70000, 7,
                      status, "", "", "", 0);
}

void testLoadOrderAndEmpty()
{
    cout << "\n===== LOAD ORDER / EMPTY =====\n";
    prepare();
    Fixture f;
    check("Invoice chua load", !f.invoices.isLoaded());
    check("Ticket chua load", !f.tickets.isLoaded());
    check("Invoice khong load truoc dependencies", !tryLoadInvoice(f.invoices));
    check("Ticket khong load truoc dependencies", !tryLoadTicket(f.tickets));
    check("Load dependencies", f.loadDependencies());
    check("Ticket khong load truoc Invoice", !tryLoadTicket(f.tickets));
    check("Invoice load file rong", tryLoadInvoice(f.invoices));
    check("Ticket load file rong", tryLoadTicket(f.tickets));
    check("Count Invoice rong", f.invoices.count() == 0);
    check("Count Ticket rong", f.tickets.count() == 0);
    check("Invoice khong load hai lan", !tryLoadInvoice(f.invoices));
    check("Ticket khong load hai lan", !tryLoadTicket(f.tickets));
    check("Find Invoice khong ton tai", f.invoices.findById("INV9999") == nullptr);
    check("Find Ticket khong ton tai", f.tickets.findById("TKT9999") == nullptr);
    check("GetAt Invoice ngoai pham vi", f.invoices.getAt(10) == nullptr);
    check("GetAt Ticket ngoai pham vi", f.tickets.getAt(10) == nullptr);
    check("Chua co ghe ban", !f.tickets.isSeatSold("SHW0001", "A01"));
}

void testLoadValidAndRelations()
{
    cout << "\n===== VALID LOAD / RELATIONS =====\n";
    prepare(VALID_INVOICE, VALID_TICKET);
    Fixture f;
    check("Dependencies", f.loadDependencies());
    check("Invoice load hop le", tryLoadInvoice(f.invoices));
    check("Ticket load hop le", tryLoadTicket(f.tickets));
    Invoice* invoice = f.invoices.findById("INV0001");
    Ticket* ticket = f.tickets.findById("TKT0001");
    check("Invoice count 1", f.invoices.count() == 1);
    check("Ticket count 1", f.tickets.count() == 1);
    check("Invoice tim dung", invoice != nullptr);
    check("Ticket tim dung", ticket != nullptr);
    check("Invoice link Showtime", invoice && invoice->getShowtime() == f.shows.findById("SHW0001"));
    check("Invoice link Seller", invoice && invoice->getSeller() == f.users.findById("USR0001"));
    check("Invoice link Member", invoice && invoice->getMember() == f.members.findById("MEM0001"));
    check("Ticket link Showtime", ticket && ticket->getShowtime() == f.shows.findById("SHW0001"));
    check("Ticket link Seat", ticket && ticket->getSeat() == f.seats.findById("ROM0001", "A01"));
    check("Ticket link Invoice", ticket && ticket->getInvoice() == invoice);
    check("Invoice co 1 ticket", invoice && invoice->getTicketCount() == 1 && invoice->getTicketAt(0) == ticket);
    check("User co 1 invoice", f.users.findById("USR0001")->getInvoiceCount() == 1);
    check("Member co 1 invoice", f.members.findById("MEM0001")->getInvoiceCount() == 1);
    check("A01 da ban", f.tickets.isSeatSold("SHW0001", "A01"));
    check("A02 chua ban", !f.tickets.isSeatSold("SHW0001", "A02"));
    check("Sai suat khong bi ban", !f.tickets.isSeatSold("SHW9999", "A01"));
    check("Loc Invoice theo suat", f.invoices.getByShowtime("SHW0001").size() == 1);
    check("Loc Invoice theo nhan vien", f.invoices.getByStaff("USR0001").size() == 1);
    check("Loc Invoice theo member", f.invoices.getByMember("MEM0001").size() == 1);
    check("Loc Invoice member rong", f.invoices.getByMember("").empty());
    check("Loc Ticket theo suat", f.tickets.getByShowtime("SHW0001").size() == 1);
    check("Loc Ticket theo invoice", f.tickets.getByInvoice("INV0001").size() == 1);
    check("Loc khong ton tai", f.tickets.getByInvoice("INV9999").empty());
    check("Save Invoice", f.invoices.save());
    check("Save Ticket", f.tickets.save());
}

void testAddInvoiceAndTicket()
{
    cout << "\n===== ADD / DUPLICATE / SOLD =====\n";
    prepare();
    Fixture f;
    check("Dependencies", f.loadDependencies());
    check("Load Invoice rong", tryLoadInvoice(f.invoices));
    check("Load Ticket rong", tryLoadTicket(f.tickets));
    check("Invoice add nullptr", !f.invoices.add(nullptr));
    check("Ticket add nullptr", !f.tickets.add(nullptr));

    Invoice* invoice = new Invoice("INV0001", "SHW0001", "USR0001",
                                   "MEM0001", "2027-01-10T09:00:00", 70000);
    bool ok = f.invoices.add(invoice);
    check("Add Invoice", ok);
    if (!ok) { delete invoice; return; }
    check("Invoice link Seller sau add", invoice->getSeller() == f.users.findById("USR0001"));
    check("Invoice link Member sau add", invoice->getMember() == f.members.findById("MEM0001"));

    Ticket* first = makeTicket("TKT0001", "A01");
    ok = f.tickets.add(first);
    check("Add Ticket", ok);
    if (!ok) { delete first; return; }
    check("A01 SOLD sau add", f.tickets.isSeatSold("SHW0001", "A01"));
    check("Invoice chua 1 ve", invoice->getTicketCount() == 1);

    Invoice* duplicateInvoice = new Invoice("INV0001", "SHW0001", "USR0001",
                                            "MEM0001", "2027-01-10T09:00:00", 70000);
    ok = f.invoices.add(duplicateInvoice);
    check("Chan trung Invoice ID", !ok);
    if (!ok) delete duplicateInvoice;

    Ticket* duplicateTicket = makeTicket("TKT0001", "A02");
    ok = f.tickets.add(duplicateTicket);
    check("Chan trung Ticket ID", !ok);
    if (!ok) delete duplicateTicket;

    Ticket* soldTwice = makeTicket("TKT0002", "A01");
    ok = f.tickets.add(soldTwice);
    check("Chan ban trung ghe", !ok);
    if (!ok) delete soldTwice;

    Ticket* second = makeTicket("TKT0002", "A02");
    ok = f.tickets.add(second);
    check("Cho ban ghe A02", ok);
    if (!ok) delete second;
    check("A02 SOLD", f.tickets.isSeatSold("SHW0001", "A02"));
}

void testCancelledHistory()
{
    cout << "\n===== CANCELLED HISTORY =====\n";
    prepare("INV0001|SHW0001|USR0001|MEM0001|2027-01-10T09:00:00|140000\n",
            "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|CANCELLED|2027-01-10T09:30:00|USR0002|Khach huy|70000\n"
            "TKT0002|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\n");
    Fixture f;
    check("Dependencies", f.loadDependencies());
    check("Invoice load", tryLoadInvoice(f.invoices));
    check("Cho phep ve CANCELLED va SOLD cung ghe", tryLoadTicket(f.tickets));
    check("Ghe A01 van SOLD", f.tickets.isSeatSold("SHW0001", "A01"));
    check("Invoice giu 2 ve lich su", f.invoices.findById("INV0001") &&
          f.invoices.findById("INV0001")->getTicketCount() == 2);
}

void testInvalidInvoiceFiles()
{
    cout << "\n===== INVALID INVOICE FILES =====\n";
    struct Case { const char* name; const char* data; };
    const Case cases[] = {
        {"Thieu cot", "INV0001|SHW0001|USR0001|MEM0001|2027-01-10T09:00:00\n"},
        {"Trung ID", "INV0001|SHW0001|USR0001||2027-01-10T09:00:00|70000\nINV0001|SHW0001|USR0001||2027-01-10T09:00:00|70000\n"},
        {"ID sai", "BAD0001|SHW0001|USR0001||2027-01-10T09:00:00|70000\n"},
        {"Showtime khong ton tai", "INV0001|SHW9999|USR0001||2027-01-10T09:00:00|70000\n"},
        {"Staff khong ton tai", "INV0001|SHW0001|USR9999||2027-01-10T09:00:00|70000\n"},
        {"Member khong ton tai", "INV0001|SHW0001|USR0001|MEM9999|2027-01-10T09:00:00|70000\n"},
        {"Tao sau gio chieu", "INV0001|SHW0001|USR0001||2027-01-10T10:00:00|70000\n"},
        {"Tong tien am", "INV0001|SHW0001|USR0001||2027-01-10T09:00:00|-1\n"},
        {"Tong tien chu", "INV0001|SHW0001|USR0001||2027-01-10T09:00:00|abc\n"}
    };
    for (const Case& c : cases)
    {
        prepare(c.data);
        Fixture f;
        if (!f.loadDependencies()) { check("Dependencies", false); continue; }
        bool ok = tryLoadInvoice(f.invoices);
        check(string("Reject Invoice: ") + c.name, !ok);
        check(string("Rollback Invoice: ") + c.name,
              !f.invoices.isLoaded() && f.invoices.count() == 0);
        check(string("No dangling User link: ") + c.name,
              f.users.findById("USR0001")->getInvoiceCount() == 0);
        check(string("No dangling Member link: ") + c.name,
              f.members.findById("MEM0001")->getInvoiceCount() == 0);
    }
}

void testInvalidTicketFiles()
{
    cout << "\n===== INVALID TICKET FILES =====\n";
    struct Case { const char* name; const char* data; };
    const Case cases[] = {
        {"Thieu cot", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000\n"},
        {"Trung ID", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\nTKT0001|INV0001|SHW0001|A02|REGULAR|70000|0|70000|7|SOLD||||0\n"},
        {"Invoice khong ton tai", "TKT0001|INV9999|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\n"},
        {"Showtime khong ton tai", "TKT0001|INV0001|SHW9999|A01|REGULAR|70000|0|70000|7|SOLD||||0\n"},
        {"Seat khong ton tai", "TKT0001|INV0001|SHW0001|Z99|REGULAR|70000|0|70000|7|SOLD||||0\n"},
        {"TicketType sai", "TKT0001|INV0001|SHW0001|A01|UNKNOWN|70000|0|70000|7|SOLD||||0\n"},
        {"Status sai", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|UNKNOWN||||0\n"},
        {"Gia khong khop", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|10000|70000|7|SOLD||||0\n"},
        {"SOLD co ly do huy", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD|2027-01-10T09:30:00|USR0002|Huy|0\n"},
        {"CANCELLED sau gio chieu", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|CANCELLED|2027-01-10T10:00:00|USR0002|Huy|70000\n"},
        {"CANCELLED truoc hoa don", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|CANCELLED|2027-01-10T08:00:00|USR0002|Huy|70000\n"},
        {"Hai SOLD cung ghe", "TKT0001|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\nTKT0002|INV0001|SHW0001|A01|REGULAR|70000|0|70000|7|SOLD||||0\n"}
    };
    for (const Case& c : cases)
    {
        prepare(VALID_INVOICE, c.data);
        Fixture f;
        if (!f.loadDependencies() || !tryLoadInvoice(f.invoices))
        {
            check("Dependencies + Invoice", false);
            continue;
        }
        bool ok = tryLoadTicket(f.tickets);
        check(string("Reject Ticket: ") + c.name, !ok);
        check(string("Rollback Ticket: ") + c.name,
              !f.tickets.isLoaded() && f.tickets.count() == 0);
        Invoice* inv = f.invoices.findById("INV0001");
        check(string("No dangling Invoice link: ") + c.name,
              inv && inv->getTicketCount() == 0);
    }
}

void testSaveReload()
{
    cout << "\n===== SAVE / RELOAD =====\n";
    prepare(VALID_INVOICE, VALID_TICKET);
    {
        Fixture f;
        check("Dependencies", f.loadDependencies());
        check("Invoice load", tryLoadInvoice(f.invoices));
        check("Ticket load", tryLoadTicket(f.tickets));
        check("Invoice save", f.invoices.save());
        check("Ticket save", f.tickets.save());
    }
    {
        Fixture f;
        check("Dependencies reload", f.loadDependencies());
        check("Invoice reload", tryLoadInvoice(f.invoices));
        check("Ticket reload", tryLoadTicket(f.tickets));
        check("Ticket giu du lieu", f.tickets.findById("TKT0001") != nullptr);
        check("Invoice giu du lieu", f.invoices.findById("INV0001") != nullptr);
        check("Lien ket sau reload", f.invoices.findById("INV0001") &&
              f.invoices.findById("INV0001")->getTicketCount() == 1);
    }
}

int main()
{
    try
    {
        testLoadOrderAndEmpty();
        testLoadValidAndRelations();
        testAddInvoiceAndTicket();
        testCancelledHistory();
        testInvalidInvoiceFiles();
        testInvalidTicketFiles();
        testSaveReload();
    }
    catch (const exception& e)
    {
        cout << "[FATAL] " << e.what() << '\n';
        failed++;
    }
    cleanup();
    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << "\nFailed: " << failed
         << "\nTotal: " << passed + failed << '\n';
    return failed == 0 ? 0 : 1;
}
