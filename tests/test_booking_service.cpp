
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

string read(const string& name)
{
    ifstream in(name, ios::binary);
    if (!in) throw runtime_error("Khong doc file test: " + name);
    return string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
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
          "ADMIN|PRICING_VIEW\n");

    write("pricing.txt",
          "STANDARD|75000\n"
          "VIP|100000\n"
          "COUPLE|160000\n"
          "STUDENT_DISCOUNT|10\n"
          "MEMBER_DISCOUNT|0\n");
}

// TEST 1: TINH TIEN VA KIEM TRA DAU VAO
void testCalculateBooking()
{
    prepare();

    Fixture f;
    f.load();

    check(f.auth.login("staff", "123456"),
          "Login Staff");

    check(f.booking.getAvailableSeats("SHW0001").size() == 3,
          "Ba ghe trong");

    BookingSummary summary;

    vector<BookingItem> items = {
        {"A01", TicketType::REGULAR},
        {"A02", TicketType::REGULAR}
    };

    check(f.booking.calculateBooking("SHW0001", items,
                                     "MEM0001", summary),
          "Tinh tien hai ve");

    check(summary.ticketPrices.size() == 2 &&
          summary.totalAmount == 150000 &&
          summary.earnedPoints == 14,
          "Tong tien va diem hai ve");

    vector<BookingItem> duplicateItems = {
        {"A01", TicketType::REGULAR},
        {"A01", TicketType::STUDENT}
    };

    check(!f.booking.calculateBooking("SHW0001", duplicateItems,
                                      "", summary) &&
          summary.ticketPrices.empty(),
          "Khong cho chon trung ghe");

    vector<BookingItem> invalidSeat = {
        {"A99", TicketType::REGULAR}
    };

    check(!f.booking.calculateBooking("SHW0001", invalidSeat,
                                      "", summary),
          "Ghe khong ton tai");

    check(!f.booking.calculateBooking("SHW0001", items,
                                      "MEM9999", summary),
          "Thanh vien khong ton tai");

    vector<BookingItem> studentItems = {
        {"A03", TicketType::STUDENT}
    };

    check(f.booking.calculateBooking("SHW0001", studentItems,
                                     "MEM0001", summary) &&
          summary.totalAmount == 90000 &&
          summary.earnedPoints == 9,
          "Gia ve hoc sinh VIP");

    vector<BookingItem> invalidType = {
        {"A03", static_cast<TicketType>(99)}
    };

    check(!f.booking.calculateBooking("SHW0001", invalidType,
                                      "", summary) &&
          summary.ticketPrices.empty(),
          "Loai ve khong hop le");

    vector<BookingItem> emptyItems;

    check(!f.booking.calculateBooking("SHW0001", emptyItems,
                                      "", summary),
          "Danh sach ve rong");

    check(!f.booking.calculateBooking("SHW9999", items,
                                      "", summary),
          "Suat chieu khong ton tai");
}

// TEST 2: BAN VE THANH CONG
void testCreateBooking()
{
    prepare();

    {
        Fixture f;
        f.load();

        check(f.auth.login("staff", "123456"),
              "Dang nhap nhan vien");

        const string originalMembers = read("members.txt");

        vector<BookingItem> items = {
            {"A01", TicketType::REGULAR},
            {"A02", TicketType::REGULAR}
        };

        string invoiceId = "OLD";

        check(!f.booking.createBooking("SHW0001", items,
                                       "MEM0001", false, true,
                                       invoiceId) &&
              invoiceId.empty(),
              "Bat buoc xac nhan tuoi");

        check(!f.booking.createBooking("SHW0001", items,
                                       "MEM0001", true, false,
                                       invoiceId),
              "Bat buoc xac nhan thanh toan");

        check(f.invoices.count() == 0 &&
              f.tickets.count() == 0,
              "Khong tao du lieu khi tu choi");

        check(f.booking.createBooking("SHW0001", items,
                                      "MEM0001", true, true,
                                      invoiceId) &&
              invoiceId == "INV0001",
              "Ban hai ve thanh cong");

        check(f.invoices.count() == 1 &&
              f.tickets.count() == 2,
              "Da tao Invoice va Ticket");

        check(f.members.findById("MEM0001")->getPoints() == 14,
              "Cong diem thanh vien");

        check(read("invoices.txt").empty() && read("tickets.txt").empty() &&
              read("members.txt") == originalMembers,
              "Ban ve chi cap nhat RAM");

        Invoice* invoice = f.invoices.findById(invoiceId);

        check(invoice != nullptr &&
              invoice->getTicketCount() == 2 &&
              invoice->getSeller() != nullptr &&
              invoice->getMember() != nullptr,
              "Lien ket Invoice hop le");

        Ticket* ticket1 = f.tickets.findById("TKT0001");
        Ticket* ticket2 = f.tickets.findById("TKT0002");

        check(ticket1 != nullptr &&
              ticket2 != nullptr &&
              ticket1->getPointsEarned() == 7 &&
              ticket2->getPointsEarned() == 7,
              "ID va diem tung Ticket");

        check(f.booking.getAvailableSeats("SHW0001").size() == 1,
              "Ghe da ban khong con trong");

        vector<BookingItem> soldSeat = {
            {"A01", TicketType::REGULAR}
        };

        check(!f.booking.createBooking("SHW0001", soldSeat,
                                       "", true, true, invoiceId),
              "Khong ban lai ghe da ban");

        check(f.invoices.count() == 1 &&
              f.tickets.count() == 2,
              "Khong phat sinh du lieu khi ban trung");

        f.auth.logout();

        bool denied = false;

        try
        {
            f.booking.createBooking("SHW0001", soldSeat,
                                    "", true, true, invoiceId);
        }
        catch (const exception&)
        {
            denied = true;
        }

        check(denied, "Chua dang nhap khong duoc ban ve");
    }

    // Chua save nen context moi doc lai du lieu ban dau.
    {
        Fixture f;
        f.load();

        check(f.invoices.count() == 0 &&
              f.tickets.count() == 0 &&
              f.members.findById("MEM0001")->getPoints() == 0,
              "Context moi chua thay giao dich RAM");

        check(!f.tickets.rollbackPending("TKT0001") &&
              !f.invoices.rollbackPending("INV0001"),
              "Khong co du lieu pending o context moi");

        check(f.auth.login("admin", "123456"),
              "Dang nhap Admin");

        vector<BookingItem> items = {
            {"A03", TicketType::REGULAR}
        };

        string invoiceId;

        check(f.booking.createBooking("SHW0001", items,
                                      "", true, true, invoiceId) &&
              invoiceId == "INV0001",
              "Admin ban ve khong co thanh vien");
    }
}

// TEST 3: ROLLBACK MOT TICKET
void testRollbackSingleTicket()
{
    prepare();

    Fixture f;
    f.load();

    Invoice* invoice = new Invoice(
        "INV0001", "SHW0001", "USR0001",
        "MEM0001", "2098-01-01T09:00:00", 75000
    );

    check(f.invoices.add(invoice),
          "Them Invoice pending");

    Ticket* ticket = new Ticket(
        "TKT0001", "INV0001", "SHW0001", "A01",
        TicketType::REGULAR, 75000, 0, 75000, 7,
        TicketStatus::SOLD, "", "", "", 0
    );

    check(f.tickets.add(ticket),
          "Them Ticket pending");

    check(!f.invoices.rollbackPending("INV0001"),
          "Khong xoa Invoice khi con Ticket");

    check(f.tickets.rollbackPending("TKT0001"),
          "Rollback Ticket");

    check(invoice->getTicketCount() == 0 &&
          f.tickets.count() == 0,
          "Da go lien ket Ticket");

    check(f.invoices.rollbackPending("INV0001"),
          "Rollback Invoice");

    check(f.invoices.count() == 0,
          "Invoice da duoc xoa");
}

// TEST 4: ROLLBACK NHIEU TICKET
void testRollbackMultipleTickets()
{
    prepare();

    Fixture f;
    f.load();

    Invoice* invoice = new Invoice(
        "INV0001", "SHW0001", "USR0001",
        "MEM0001", "2098-01-01T09:00:00", 150000
    );

    check(f.invoices.add(invoice),
          "Rollback nhieu ve: them Invoice");

    Ticket* ticket1 = new Ticket(
        "TKT0001", "INV0001", "SHW0001", "A01",
        TicketType::REGULAR, 75000, 0, 75000, 7,
        TicketStatus::SOLD, "", "", "", 0
    );

    Ticket* ticket2 = new Ticket(
        "TKT0002", "INV0001", "SHW0001", "A02",
        TicketType::REGULAR, 75000, 0, 75000, 7,
        TicketStatus::SOLD, "", "", "", 0
    );

    check(f.tickets.add(ticket1),
          "Rollback nhieu ve: them Ticket 1");

    check(f.tickets.add(ticket2),
          "Rollback nhieu ve: them Ticket 2");

    check(invoice->getTicketCount() == 2,
          "Invoice lien ket hai Ticket");

    // Pending IDs phai rollback theo thu tu nguoc
    check(!f.tickets.rollbackPending("TKT0001"),
          "Khong rollback sai thu tu");

    check(f.tickets.rollbackPending("TKT0002"),
          "Rollback Ticket 2");

    check(f.tickets.rollbackPending("TKT0001"),
          "Rollback Ticket 1");

    check(invoice->getTicketCount() == 0,
          "Da go tat ca lien ket Ticket");

    check(f.invoices.rollbackPending("INV0001"),
          "Rollback Invoice sau Ticket");

    check(f.tickets.count() == 0 &&
          f.invoices.count() == 0,
          "Khong con du lieu pending");

    check(f.members.findById("MEM0001")->getPoints() == 0,
          "Diem thanh vien khong thay doi");

    check(!f.tickets.isSeatSold("SHW0001", "A01") &&
          !f.tickets.isSeatSold("SHW0001", "A02"),
          "Hai ghe tro lai trang thai chua ban");

    check(f.booking.getAvailableSeats("SHW0001").size() == 3,
          "Danh sach ghe trong duoc khoi phuc");
}

// TEST 5: BAN VE CHI CAP NHAT RAM
void testSaleWithoutWritableFiles()
{
    prepare();
    Fixture f;
    f.load();
    check(f.auth.login("staff", "123456"), "Login Staff");
    for (const string& name : {"invoices.txt", "tickets.txt", "members.txt"})
    {
        fs::remove(name);
        fs::create_directory(name);
    }
    string invoiceId;
    check(f.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}},
                                  "MEM0001", true, true, invoiceId) &&
          invoiceId == "INV0001", "Ban ve khong can ghi TXT");
    check(f.invoices.count() == 1 && f.tickets.count() == 1 &&
          f.members.findById("MEM0001")->getPoints() == 7,
          "Invoice Ticket Member cap nhat RAM");
    check(f.booking.createBooking("SHW0001", {{"A02", TicketType::REGULAR}},
                                  "", true, true, invoiceId) &&
          invoiceId == "INV0002", "Giao dich tiep theo khong bi chan");
    for (const string& name : {"invoices.txt", "tickets.txt", "members.txt"})
    {
        fs::remove(name);
    }
}
int main()
{
    fs::path old = fs::current_path();

    fs::path dir = fs::temp_directory_path() /
        ("pbl2_booking_service_test_" +
         to_string(chrono::steady_clock::now()
                       .time_since_epoch().count()));

    fs::create_directories(dir);
    fs::current_path(dir);

    try
    {
        testCalculateBooking();
        testCreateBooking();
        testRollbackSingleTicket();
        testRollbackMultipleTickets();
        testSaleWithoutWritableFiles();
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] " << e.what() << '\n';
        failed++;
    }

    fs::current_path(old);
    fs::remove_all(dir);

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "Total: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
