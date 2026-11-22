#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/business/ReportService.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/SeatRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/MemberRepository.h"
#include "../src/data_access/RolePermissionRepository.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool ok, const string& name)
{
    cout << (ok ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (ok) passed++;
    else failed++;
}

template <typename Action>
bool throwsError(Action action)
{
    try { action(); }
    catch (const exception&) { return true; }
    return false;
}

void writeFile(const string& name, const string& contents)
{
    ofstream out(name);
    if (!out || !(out << contents))
    {
        throw runtime_error("Khong ghi duoc file test: " + name);
    }
}

string readFile(const string& name)
{
    ifstream in(name);
    return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
}

void prepare(bool reportPermission = true)
{
    writeFile("movies.txt",
              "MOV0001|Movie One|Action|120|0|2020-01-01|2099-12-31|1\n"
              "MOV0002|Movie Two|Action|120|0|2020-01-01|2099-12-31|1\n"
              "MOV0003|Movie Three|Action|120|0|2020-01-01|2099-12-31|1\n"
              "MOV0004|No Sales|Action|120|0|2020-01-01|2099-12-31|1\n");
    writeFile("rooms.txt", "ROM0001|Room|4|5|1\n");
    writeFile("seats.txt",
              "ROM0001|A01|1|1|STANDARD\n"
              "ROM0001|A02|1|2|STANDARD\n"
              "ROM0001|A03|1|3|STANDARD\n"
              "ROM0001|A04|1|4|STANDARD\n"
              "ROM0001|A05|1|5|STANDARD\n");
    writeFile("shows.txt",
              "SHW0001|MOV0001|ROM0001|2099-01-01T10:00:00|2099-01-01T12:00:00|SCHEDULED\n"
              "SHW0002|MOV0002|ROM0001|2099-01-02T10:00:00|2099-01-02T12:00:00|SCHEDULED\n"
              "SHW0003|MOV0003|ROM0001|2099-01-03T10:00:00|2099-01-03T12:00:00|SCHEDULED\n");
    writeFile("users.txt",
              "USR0001|staff|123456|Staff|STAFF|1\n"
              "USR0002|admin|123456|Admin|ADMIN|1\n");
    writeFile("members.txt", "");
    writeFile("invoices.txt",
              "INV0001|SHW0001|USR0001||2026-11-20T10:00:00|200\n"
              "INV0002|SHW0002|USR0001||2026-11-22T11:00:00|80\n"
              "INV0003|SHW0003|USR0001||2026-11-22T12:00:00|80\n"
              "INV0004|SHW0001|USR0001||2026-10-31T10:00:00|50\n");
    writeFile("tickets.txt",
              "TKT0001|INV0001|SHW0001|A01|REGULAR|100|0|100|0|SOLD||||0\n"
              "TKT0002|INV0001|SHW0001|A02|REGULAR|100|0|100|0|CANCELLED|2026-11-22T09:00:00|USR0002|Refund|100\n"
              "TKT0003|INV0002|SHW0002|A03|REGULAR|80|0|80|0|SOLD||||0\n"
              "TKT0004|INV0003|SHW0003|A04|REGULAR|80|0|80|0|SOLD||||0\n"
              "TKT0005|INV0004|SHW0001|A05|REGULAR|50|0|50|0|CANCELLED|2026-11-22T13:00:00|USR0002|Refund|50\n");
    writeFile("permissions.txt", reportPermission ? "ADMIN|REPORT_VIEW\n" : "");
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
    AuthService auth{users, permissions};
    ReportService report{invoices, tickets, movies, shows, auth};

    void loadBase()
    {
        if (!(movies.load() && rooms.load() && seats.load() && shows.load() &&
              users.load() && members.load() && permissions.load()))
        {
            throw runtime_error("Khong load duoc du lieu nen");
        }
    }

    void loadAll()
    {
        loadBase();
        if (!(invoices.load() && tickets.load()))
        {
            throw runtime_error("Khong load duoc Invoice/Ticket");
        }
    }
};

void testReports()
{
    prepare();
    string invoicesBefore = readFile("invoices.txt");
    string ticketsBefore = readFile("tickets.txt");
    Fixture f;
    f.loadAll();
    check(f.auth.login("admin", "123456"), "Admin login");
    const string day20 = "2026-11-20T00:00:00";
    const string day22 = "2026-11-22T00:00:00";
    const string day23 = "2026-11-23T00:00:00";
    const string monthStart = "2026-11-01T00:00:00";
    const string monthEnd = "2026-12-01T00:00:00";

    check(f.report.getTotalSales(day22, day23) == 160, "sales by invoice date");
    check(f.report.getTotalRefund(day22, day23) == 150, "refund by cancellation date");
    check(f.report.getActualRevenue(day22, day23) == 10, "daily net revenue");
    check(f.report.getActualRevenue(day22, "2026-11-22T10:00:00") == -100,
          "refund of an older sale can make daily revenue negative");
    check(f.report.getTotalSales(day20, day22) == 200 &&
          f.report.getTotalRefund(day20, day22) == 0,
          "sale and refund on different days");
    check(f.report.getTotalSales(monthStart, monthEnd) == 360 &&
          f.report.getTotalRefund(monthStart, monthEnd) == 150 &&
          f.report.getActualRevenue(monthStart, monthEnd) == 210,
          "monthly cash flow");
    check(f.report.getTotalSales("2026-10-01T00:00:00", monthStart) == 50,
          "exclusive month upper bound");

    check(f.report.getSoldTicketCount(day22, day23) == 2,
          "tickets sold on day 22");
    check(f.report.getSoldTicketCount(monthStart, monthEnd) == 4,
          "sold count includes later cancelled ticket");
    check(f.report.getCancelledTicketCount(day22, day23) == 2 &&
          f.report.getCancelledTicketCount(day20, day22) == 0,
          "cancel count uses cancellation time");
    check(f.report.getCancelledTicketCount(monthStart, monthEnd) == 2,
          "monthly cancelled count");

    check(f.report.getMovieRevenue("MOV0001", monthStart, monthEnd) == 100,
          "movie revenue excludes cancelled tickets");
    check(f.report.getMovieRevenue("MOV0002", day22, day23) == 80 &&
          f.report.getMovieRevenue("MOV0003", day22, day23) == 80,
          "movie revenue by sale period");
    check(f.report.getMovieRevenue("MOV0004", monthStart, monthEnd) == 0,
          "valid movie without sales returns zero");
    vector<Movie*> top = f.report.getTopMovies(monthStart, monthEnd, 10);
    check(top.size() == 3 && top[0] == f.movies.findById("MOV0001") &&
          top[1] == f.movies.findById("MOV0002") &&
          top[2] == f.movies.findById("MOV0003"),
          "top movies: revenue descending, ID ascending on tie, borrowed pointers");
    check(f.report.getTopMovies(day22, day23, 1).size() == 1 &&
          f.report.getTopMovies(day22, day23, 1)[0]->getMovieId() == "MOV0002",
          "top limit and tied revenue");
    check(f.report.getTopMovies("2026-12-01T00:00:00", "2027-01-01T00:00:00", 5).empty() &&
          f.report.getTotalSales("2026-12-01T00:00:00", "2027-01-01T00:00:00") == 0,
          "empty period returns empty top and zero sales");
    check(readFile("invoices.txt") == invoicesBefore &&
          readFile("tickets.txt") == ticketsBefore,
          "reports do not write TXT");
}

void testErrors()
{
    prepare();
    Fixture f;
    f.loadAll();
    const string from = "2026-11-01T00:00:00";
    const string to = "2026-12-01T00:00:00";
    check(throwsError([&] { f.report.getTotalSales(from, to); }), "guest denied");
    f.auth.login("staff", "123456");
    check(throwsError([&] { f.report.getTotalRefund(from, to); }), "Staff denied");
    f.auth.login("admin", "123456");
    check(throwsError([&] { f.report.getTotalSales("bad", to); }),
          "invalid date rejected");
    check(throwsError([&] { f.report.getSoldTicketCount(to, from); }),
          "reversed interval rejected");
    check(throwsError([&] { f.report.getTopMovies(from, to, 0); }),
          "zero top limit rejected");
    check(throwsError([&] { f.report.getTopMovies(from, to, -1); }),
          "negative top limit rejected");
    check(throwsError([&] { f.report.getMovieRevenue("MOV9999", from, to); }),
          "unknown movie rejected");
    f.tickets.findById("TKT0001")->unlinkReferences();
    check(throwsError([&] { f.report.getCancelledTicketCount(from, to); }),
          "broken Ticket-Invoice link rejected");

    prepare(false);
    Fixture missingRight;
    missingRight.loadAll();
    missingRight.auth.login("admin", "123456");
    check(throwsError([&] { missingRight.report.getTotalSales(from, to); }),
          "Admin without REPORT_VIEW denied");

    prepare();
    Fixture notLoaded;
    notLoaded.loadBase();
    notLoaded.auth.login("admin", "123456");
    check(throwsError([&] { notLoaded.report.getTotalSales(from, to); }),
          "InvoiceRepository not loaded");
    check(notLoaded.invoices.load(), "load invoices for partial fixture");
    check(throwsError([&] { notLoaded.report.getTotalRefund(from, to); }),
          "TicketRepository not loaded");

    prepare();
    string invoices = readFile("invoices.txt");
    size_t amountPos = invoices.find("2026-11-20T10:00:00|200");
    if (amountPos == string::npos)
    {
        throw runtime_error("Khong tim thay tong tien fixture");
    }
    invoices.replace(amountPos, string("2026-11-20T10:00:00|200").size(),
                     "2026-11-20T10:00:00|201");
    writeFile("invoices.txt", invoices);
    Fixture wrongTotal;
    wrongTotal.loadAll();
    wrongTotal.auth.login("admin", "123456");
    check(throwsError([&] { wrongTotal.report.getTotalRefund(from, to); }),
          "invoice total inconsistent with tickets rejected");
}

int main()
{
    fs::path oldPath = fs::current_path();
    fs::path testPath = fs::temp_directory_path() /
        ("pbl2_report_service_" + to_string(fs::file_time_type::clock::now().time_since_epoch().count()));
    try
    {
        fs::create_directories(testPath);
        fs::current_path(testPath);
        testReports();
        testErrors();
        fs::current_path(oldPath);
        fs::remove_all(testPath);
    }
    catch (const exception& error)
    {
        fs::current_path(oldPath);
        fs::remove_all(testPath);
        cerr << "[ERROR] " << error.what() << '\n';
        return 1;
    }
    cout << "PASS: " << passed << ", FAIL: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}
