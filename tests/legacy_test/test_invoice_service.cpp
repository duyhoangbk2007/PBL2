#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../src/business/InvoiceService.h"
#include "../src/data_access/MovieRepository.h"
#include "../src/data_access/RoomRepository.h"
#include "../src/data_access/SeatRepository.h"
#include "../src/data_access/ShowtimeRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/MemberRepository.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/utils/DateTime.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (condition) passed++;
    else failed++;
}

template <typename Action>
bool throwsError(Action action)
{
    try
    {
        action();
    }
    catch (const exception&)
    {
        return true;
    }
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
    InvoiceService service{invoices, tickets, auth};

    void loadBase()
    {
        if (!(movies.load() && rooms.load() && seats.load() && shows.load() &&
              users.load() && members.load() && permissions.load()))
        {
            throw runtime_error("Khong nap duoc du lieu nen");
        }
    }

    void loadAll()
    {
        loadBase();
        if (!(invoices.load() && tickets.load()))
        {
            throw runtime_error("Khong nap duoc hoa don hoac ve");
        }
    }
};

void prepare(const string& now, bool adminPermission = true,
             bool staffPermission = true, bool orphanInvoice = false,
             bool mismatchedTotal = false)
{
    writeFile("movies.txt", "MOV0001|Phim|Action|120|13|2020-01-01|2099-12-31|1\n");
    writeFile("rooms.txt", "ROM0001|Phong|4|5|1\n");
    writeFile("seats.txt",
              "ROM0001|A01|1|1|STANDARD\n"
              "ROM0001|A02|1|2|STANDARD\n"
              "ROM0001|A03|1|3|STANDARD\n"
              "ROM0001|A04|1|4|STANDARD\n"
              "ROM0001|A05|1|5|STANDARD\n");
    writeFile("shows.txt",
              "SHW0001|MOV0001|ROM0001|2099-01-01T10:00:00|2099-01-01T12:00:00|SCHEDULED\n");
    writeFile("users.txt",
              "USR0001|staff1|123456|Staff One|STAFF|1\n"
              "USR0002|admin|123456|Admin|ADMIN|1\n"
              "USR0003|staff2|123456|Staff Two|STAFF|1\n");
    writeFile("members.txt", "MEM0001|Member|0912345678|0\n");

    string invoices =
        "INV0001|SHW0001|USR0001|MEM0001|" + now + "|" +
        (mismatchedTotal ? "125001.50" : "125000.50") + "\n" +
        "INV0002|SHW0001|USR0001||" + now + "|60000\n" +
        "INV0003|SHW0001|USR0003||" + now + "|45000\n" +
        "INV0004|SHW0001|USR0001||2025-01-01T10:00:00|30000\n";
    if (orphanInvoice)
    {
        invoices += "INV0005|SHW0001|USR0001||" + now + "|0\n";
    }
    writeFile("invoices.txt", invoices);

    writeFile("tickets.txt",
              "TKT0001|INV0001|SHW0001|A01|REGULAR|75000.25|0|75000.25|7|SOLD||||0\n"
              "TKT0002|INV0001|SHW0001|A02|REGULAR|50000.25|0|50000.25|5|SOLD||||0\n"
              "TKT0003|INV0002|SHW0001|A03|REGULAR|60000|0|60000|6|CANCELLED|" + now + "|USR0002|Khach huy|60000\n"
              "TKT0004|INV0003|SHW0001|A04|REGULAR|45000|0|45000|4|SOLD||||0\n"
              "TKT0005|INV0004|SHW0001|A05|REGULAR|30000|0|30000|3|SOLD||||0\n");

    string permissions;
    if (adminPermission) permissions += "ADMIN|INVOICE_VIEW_ALL\n";
    if (staffPermission) permissions += "STAFF|INVOICE_VIEW_OWN\n";
    writeFile("permissions.txt", permissions);
}

void testQueries(const string& now)
{
    prepare(now);
    string invoiceText = readFile("invoices.txt");
    string ticketText = readFile("tickets.txt");
    Fixture f;
    f.loadAll();
    check(f.auth.login("admin", "123456"), "Admin login");
    check(f.service.findInvoiceById("INV0001") == f.invoices.findById("INV0001"),
          "find returns repository pointer");
    check(f.service.findInvoiceById("INV9999") == nullptr, "unknown invoice returns nullptr");

    string today = now.substr(0, 10) + "T00:00:00";
    string tomorrow = DateTime::addMinutes(today, 1440);
    vector<Invoice*> all = f.service.getInvoiceHistory(today, tomorrow);
    check(all.size() == 3 && all[0]->getInvoiceId() == "INV0001" &&
          all[1]->getInvoiceId() == "INV0002" && all[2]->getInvoiceId() == "INV0003",
          "time filter and equal-time ID ordering");
    check(f.service.getInvoiceHistory(today, now).empty(), "exclusive upper bound");
    check(f.service.getInvoiceHistory(today, tomorrow, "USR0001").size() == 2,
          "staff filter");
    check(f.service.getInvoiceHistory(today, tomorrow, "", "MEM0001").size() == 1,
          "member filter");
    check(f.service.getInvoiceHistory(today, tomorrow, "USR0003", "MEM0001").empty(),
          "combined filters");
    check(f.service.getInvoiceHistory("2025-01-01T00:00:00", tomorrow).size() == 4,
          "Admin sees older invoices");
    check(throwsError([&] { f.service.getInvoiceHistory("bad", tomorrow); }),
          "invalid date rejected");
    check(throwsError([&] { f.service.getInvoiceHistory(tomorrow, today); }),
          "reversed interval rejected");

    check(f.service.getInvoiceStatus("INV0001") == InvoiceStatus::ACTIVE,
          "all sold is ACTIVE");
    check(f.service.getInvoiceStatus("INV0002") == InvoiceStatus::CANCELLED,
          "all cancelled is CANCELLED");
    check(f.service.getRefundAmount("INV0001") == 0,
          "sold invoice refund zero");
    check(f.service.getRefundAmount("INV0002") == 60000 &&
          f.service.getRemainingAmount("INV0002") == 0,
          "full refund and zero remaining");
    check(f.service.getRemainingAmount("INV0001") == 125000.50,
          "remaining retains fractional amount");
    f.tickets.findById("TKT0001")->markCancelled(now, "USR0002", "Khach huy");
    check(f.service.getInvoiceStatus("INV0001") == InvoiceStatus::PARTIALLY_CANCELLED,
          "mixed tickets are PARTIALLY_CANCELLED");
    check(f.service.getRefundAmount("INV0001") == 75000.25 &&
          f.service.getRemainingAmount("INV0001") == 50000.25,
          "partial refund uses ticket snapshot");
    check(throwsError([&] { f.service.getInvoiceStatus("INV9999"); }),
          "missing invoice is error for status");
    check(readFile("invoices.txt") == invoiceText &&
          readFile("tickets.txt") == ticketText,
          "queries do not write TXT");
}

void testPermissions(const string& now)
{
    prepare(now);
    Fixture f;
    f.loadAll();
    string today = now.substr(0, 10) + "T00:00:00";
    string tomorrow = DateTime::addMinutes(today, 1440);
    check(throwsError([&] { f.service.findInvoiceById("INV0001"); }),
          "guest cannot view");
    check(f.auth.login("staff1", "123456"), "Staff login");
    check(f.service.findInvoiceById("INV0004") != nullptr,
          "Staff can find own old invoice by ID");
    check(throwsError([&] { f.service.findInvoiceById("INV0003"); }),
          "Staff cannot see another seller's invoice");
    check(f.service.getInvoiceHistory(today, tomorrow).size() == 2,
          "Staff lists only own invoices today");
    check(throwsError([&] { f.service.getInvoiceHistory("2025-01-01T00:00:00", tomorrow); }),
          "Staff cannot list old interval");
    check(throwsError([&] { f.service.getInvoiceHistory(today, "2099-01-01T00:00:00"); }),
          "Staff cannot list future interval");
    check(throwsError([&] { f.service.getInvoiceHistory(today, tomorrow, "USR0003"); }),
          "Staff cannot override seller filter");
    check(throwsError([&] { f.service.getRefundAmount("INV0003"); }),
          "Staff cannot compute another seller's refund");
}

void testErrors(const string& now)
{
    prepare(now, false, true);
    Fixture noAdminRight;
    noAdminRight.loadAll();
    noAdminRight.auth.login("admin", "123456");
    check(throwsError([&] { noAdminRight.service.findInvoiceById("INV0001"); }),
          "Admin without RBAC permission denied");

    prepare(now, true, false);
    Fixture noStaffRight;
    noStaffRight.loadAll();
    noStaffRight.auth.login("staff1", "123456");
    check(throwsError([&] { noStaffRight.service.getInvoiceHistory(now, DateTime::addMinutes(now, 1)); }),
          "Staff without RBAC permission denied");

    prepare(now);
    Fixture notLoaded;
    notLoaded.loadBase();
    notLoaded.auth.login("admin", "123456");
    check(throwsError([&] { notLoaded.service.findInvoiceById("INV0001"); }),
          "unloaded InvoiceRepository rejected");
    check(notLoaded.invoices.load(), "load invoices separately");
    check(throwsError([&] { notLoaded.service.getInvoiceStatus("INV0001"); }),
          "unloaded TicketRepository rejected");

    prepare(now, true, true, true);
    Fixture orphan;
    orphan.loadAll();
    orphan.auth.login("admin", "123456");
    check(throwsError([&] { orphan.service.getInvoiceStatus("INV0005"); }),
          "invoice without tickets rejected");

    prepare(now, true, true, false, true);
    Fixture mismatch;
    mismatch.loadAll();
    mismatch.auth.login("admin", "123456");
    check(throwsError([&] { mismatch.service.getRemainingAmount("INV0001"); }),
          "invoice total mismatch rejected");

    prepare(now);
    Fixture brokenLink;
    brokenLink.loadAll();
    brokenLink.auth.login("admin", "123456");
    brokenLink.tickets.findById("TKT0001")->unlinkReferences();
    check(throwsError([&] { brokenLink.service.getRefundAmount("INV0001"); }),
          "broken Ticket-Invoice link rejected");
}

int main()
{
    fs::path oldPath = fs::current_path();
    fs::path testPath = fs::temp_directory_path() /
        ("pbl2_invoice_service_" + to_string(fs::file_time_type::clock::now().time_since_epoch().count()));
    try
    {
        fs::create_directories(testPath);
        fs::current_path(testPath);
        string now = DateTime::getCurrentDateTime();
        testQueries(now);
        testPermissions(now);
        testErrors(now);
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
