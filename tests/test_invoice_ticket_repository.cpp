// Test gop InvoiceRepository + TicketRepository (refactor/hoang-backend)
// Chi dung file TXT tam, khong cham vao du lieu that.
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/data_access/IRepository.h"
#include "../src/data_access/InvoiceRepository.h"
#include "../src/data_access/TicketRepository.h"
#include "../src/models/Showtime.h"
#include "../src/models/Seat.h"
#include "../src/models/User.h"
#include "../src/models/Member.h"

using namespace std;
namespace fs = std::filesystem;

// Dependency nhe cho test: cung cap du lieu da load qua IRepository<T>.
// Khong kiem thu cac Repository phu thuoc o day.
template <typename T>
class FakeRepository : public IRepository<T>
{
private:
    vector<T*> items;
    function<string(T*)> getId;
    bool loaded = true;
public:
    explicit FakeRepository(function<string(T*)> getId) : getId(getId) {}
    bool load() override { loaded = true; return true; }
    bool save() override { return loaded; }
    bool add(T* item) override
    {
        if (!loaded || item == nullptr || findById(getId(item))) return false;
        items.push_back(item);
        return true;
    }
    bool update(const T&) override { return false; }
    bool removeById(const string&) override { return false; }
    T* findById(const string& id) override
    {
        for (T* item : items) if (getId(item) == id) return item;
        return nullptr;
    }
    T* getAt(int index) override
    {
        return index >= 0 && index < static_cast<int>(items.size()) ? items[index] : nullptr;
    }
    int count() override { return static_cast<int>(items.size()); }
    bool isLoaded() override { return loaded; }
    void setLoaded(bool value) { loaded = value; }
};

void require(bool condition, const string& message)
{
    if (!condition) throw runtime_error(message);
}

void writeFile(const fs::path& path, const string& content)
{
    ofstream out(path, ios::binary | ios::trunc);
    require(static_cast<bool>(out), "Khong mo duoc file test");
    out << content;
    require(static_cast<bool>(out), "Khong ghi duoc file test");
}

struct Fixture
{
    fs::path dir;
    fs::path invoicePath;
    fs::path ticketPath;
    Showtime showtime;
    Seat seatA;
    Seat seatB;
    User staff;
    Member member;
    FakeRepository<Showtime> showtimes;
    FakeRepository<Seat> seats;
    FakeRepository<User> users;
    FakeRepository<Member> members;
    InvoiceRepository invoices;
    TicketRepository tickets;

    explicit Fixture(int number)
        : dir(fs::temp_directory_path() / ("pbl2_invoice_ticket_test_" + to_string(number))),
          invoicePath(dir / "invoices.txt"), ticketPath(dir / "tickets.txt"),
          showtime("SHW0001", "MOV0001", "ROM0001", "2026-12-20T18:00:00", "2026-12-20T20:00:00", ShowtimeStatus::SCHEDULED),
          seatA("ROM0001", "A1", 1, 1, SeatType::STANDARD),
          seatB("ROM0001", "A2", 1, 2, SeatType::STANDARD),
          staff("USR0001", "staff_test", "password", "Nhan vien test", UserRole::STAFF, true),
          member("MEM0001", "Khach hang test", "0912345678", 0),
          showtimes([](Showtime* x) { return x->getShowtimeId(); }),
          seats([](Seat* x) { return x->getRoomId() + "|" + x->getSeatId(); }),
          users([](User* x) { return x->getUserId(); }),
          members([](Member* x) { return x->getMemberId(); }),
          invoices(invoicePath.string(), showtimes, users, members),
          tickets(ticketPath.string(), showtimes, seats, invoices)
    {
        fs::create_directories(dir);
        writeFile(invoicePath, "");
        writeFile(ticketPath, "");
        require(showtimes.add(&showtime), "Fixture: showtime");
        require(seats.add(&seatA) && seats.add(&seatB), "Fixture: seats");
        require(users.add(&staff) && members.add(&member), "Fixture: user/member");
    }

    ~Fixture()
    {
        // TicketRepository va InvoiceRepository tu huy SAU than destructor nay;
        // file tam chi bi xoa, khong anh huong cac doi tuong trong bo nho.
        error_code ec;
        fs::remove_all(dir, ec);
    }

    void loadEmpty()
    {
        require(invoices.load(), "Invoice load empty");
        require(tickets.load(), "Ticket load empty");
    }

    Invoice* makeInvoice(const string& id = "INV0001", const string& memberId = "MEM0001")
    {
        return new Invoice(id, "SHW0001", "USR0001", memberId, "2026-12-19T10:00:00", 200000);
    }

    Ticket* makeTicket(const string& id = "TKT0001", const string& seatId = "A1",
                       const string& invoiceId = "INV0001", int points = 0,
                       TicketStatus status = TicketStatus::SOLD)
    {
        const bool cancelled = status == TicketStatus::CANCELLED;
        return new Ticket(id, invoiceId, "SHW0001", seatId, TicketType::REGULAR,
                          100000, 10000, 90000, points, status,
                          cancelled ? "2026-12-19T12:00:00" : "",
                          cancelled ? "USR0001" : "",
                          cancelled ? "Khach yeu cau" : "",
                          cancelled ? 90000 : 0);
    }
};

int main()
{
    int passed = 0, failed = 0, nextFixture = 0;
    auto test = [&](const string& name, const function<void(Fixture&)>& body)
    {
        try
        {
            Fixture f(++nextFixture);
            body(f);
            cout << "[PASS] " << name << '\n';
            ++passed;
        }
        catch (const exception& e)
        {
            cout << "[FAIL] " << name << ": " << e.what() << '\n';
            ++failed;
        }
        catch (...)
        {
            cout << "[FAIL] " << name << ": exception khong xac dinh\n";
            ++failed;
        }
    };

    test("Load rong va truy van", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.isLoaded() && f.tickets.isLoaded(), "loaded");
        require(f.invoices.count() == 0 && f.tickets.count() == 0, "count");
        require(f.invoices.getAt(0) == nullptr && f.tickets.getAt(-1) == nullptr, "getAt");
        require(!f.invoices.load() && !f.tickets.load(), "load lan hai");
    });

    test("Invoice add, link Seller/Member/Showtime", [](Fixture& f) {
        f.loadEmpty();
        Invoice* invoice = f.makeInvoice();
        require(f.invoices.add(invoice), "add invoice");
        require(f.invoices.findById("INV0001") == invoice, "find invoice");
        require(invoice->getShowtime() == &f.showtime && invoice->getSeller() == &f.staff &&
                invoice->getMember() == &f.member, "link invoice");
        require(f.staff.getInvoiceCount() == 1 && f.member.getInvoiceCount() == 1, "link hai chieu");
        require(!f.invoices.add(invoice), "trung con tro");
        Invoice* duplicate = f.makeInvoice();
        require(!f.invoices.add(duplicate), "trung ID");
        delete duplicate;
        require(!f.invoices.add(nullptr), "nullptr");
        require(f.invoices.count() == 1, "count sau add loi");
    });

    test("Invoice khong hop le va tham chieu sai", [](Fixture& f) {
        f.loadEmpty();
        Invoice* badMember = f.makeInvoice("INV0002", "MEM9999");
        require(!f.invoices.add(badMember), "member khong ton tai");
        delete badMember;
        Invoice* badShowtime = new Invoice("INV0003", "SHW9999", "USR0001", "", "2026-12-19T10:00:00", 100);
        require(!f.invoices.add(badShowtime), "showtime khong ton tai");
        delete badShowtime;
        Invoice* badTime = new Invoice("INV0004", "SHW0001", "USR0001", "", "2026-12-21T10:00:00", 100);
        require(!f.invoices.add(badTime), "hoa don sau suat chieu");
        delete badTime;
        require(f.invoices.count() == 0, "khong them du lieu loi");
    });

    test("Ticket add, link va chan trung ghe SOLD", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        Ticket* first = f.makeTicket();
        require(f.tickets.add(first), "ticket add");
        require(first->getInvoice() == f.invoices.findById("INV0001") &&
                first->getSeat() == &f.seatA && first->getShowtime() == &f.showtime, "ticket link");
        require(first->getInvoice()->getTicketCount() == 1 &&
                first->getInvoice()->getTicketAt(0) == first, "invoice link ticket");
        Ticket* occupied = f.makeTicket("TKT0002", "A1");
        require(!f.tickets.add(occupied), "ghe SOLD trung");
        delete occupied;
        Ticket* second = f.makeTicket("TKT0002", "A2");
        require(f.tickets.add(second), "ghe khac");
        require(f.tickets.count() == 2 && first->getInvoice()->getTicketCount() == 2, "hai ve");
    });

    test("Ticket trung ID, trung con tro, nullptr", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        Ticket* ticket = f.makeTicket();
        require(f.tickets.add(ticket), "add");
        require(!f.tickets.add(ticket), "trung con tro");
        Ticket* duplicate = f.makeTicket("TKT0001", "A2");
        require(!f.tickets.add(duplicate), "trung ID");
        delete duplicate;
        require(!f.tickets.add(nullptr), "nullptr");
        require(f.tickets.count() == 1 && ticket->getInvoice()->getTicketCount() == 1, "khong mat link");
    });

    test("Ticket points > 0 phai co Member", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice("INV0001", "")), "invoice khong member");
        Ticket* invalid = f.makeTicket("TKT0001", "A1", "INV0001", 5);
        require(!f.tickets.add(invalid), "khong duoc tich diem");
        delete invalid;
        require(f.invoices.findById("INV0001")->getTicketCount() == 0, "khong link loi");
        Ticket* valid = f.makeTicket("TKT0002", "A1", "INV0001", 0);
        require(f.tickets.add(valid), "khong diem duoc phep");
    });

    test("Ticket tham chieu sai va gia sai", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        Ticket* missingSeat = f.makeTicket("TKT0002", "Z9");
        require(!f.tickets.add(missingSeat), "seat khong ton tai");
        delete missingSeat;
        Ticket* missingInvoice = f.makeTicket("TKT0003", "A1", "INV9999");
        require(!f.tickets.add(missingInvoice), "invoice khong ton tai");
        delete missingInvoice;
        Ticket* badPrice = new Ticket("TKT0004", "INV0001", "SHW0001", "A1", TicketType::REGULAR,
                                      100000, 10000, 100000, 0, TicketStatus::SOLD, "", "", "", 0);
        require(!f.tickets.add(badPrice), "paid khac base-discount");
        delete badPrice;
    });

    test("Ticket update SOLD sang CANCELLED", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        require(f.tickets.add(f.makeTicket()), "ticket");
        Ticket cancelled("TKT0001", "INV0001", "SHW0001", "A1", TicketType::REGULAR,
                         100000, 10000, 90000, 0, TicketStatus::CANCELLED,
                         "2026-12-19T12:00:00", "USR0001", "Khach yeu cau", 90000);
        require(f.tickets.update(cancelled), "update");
        Ticket* stored = f.tickets.findById("TKT0001");
        require(stored->getStatus() == TicketStatus::CANCELLED, "status");
        require(stored->getRefundAmount() == 90000 &&
                stored->getCancelledBy() == "USR0001", "refund / cancelledBy");
        require(!f.tickets.update(cancelled), "khong huy lan hai");
    });

    test("Ticket update khong duoc sua truong bat bien", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        require(f.tickets.add(f.makeTicket()), "ticket");
        Ticket changed("TKT0001", "INV0001", "SHW0001", "A1", TicketType::STUDENT,
                       100000, 10000, 90000, 0, TicketStatus::CANCELLED,
                       "2026-12-19T12:00:00", "USR0001", "Khach yeu cau", 90000);
        require(!f.tickets.update(changed), "doi ticketType");
        require(f.tickets.findById("TKT0001")->getStatus() == TicketStatus::SOLD, "giu SOLD");
    });

    test("Invoice khong xoa khi con Ticket", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        require(f.tickets.add(f.makeTicket()), "ticket");
        require(!f.invoices.removeById("INV0001"), "chan xoa invoice");
        require(f.tickets.removeById("TKT0001"), "rollback ticket");
        require(f.invoices.findById("INV0001")->getTicketCount() == 0, "unlink ticket");
        require(f.invoices.removeById("INV0001"), "xoa invoice");
        require(f.staff.getInvoiceCount() == 0 && f.member.getInvoiceCount() == 0, "unlink invoice");
    });

    test("Save Invoice va Ticket", [](Fixture& f) {
        f.loadEmpty();
        require(f.invoices.add(f.makeInvoice()), "invoice");
        require(f.tickets.add(f.makeTicket()), "ticket");
        require(f.invoices.save() && f.tickets.save(), "save");
        require(fs::file_size(f.invoicePath) > 0 && fs::file_size(f.ticketPath) > 0, "file co du lieu");
    });

    test("Load Invoice + Ticket tu TXT va lien ket hai chieu", [](Fixture& f) {
        writeFile(f.invoicePath, "INV0001|SHW0001|USR0001|MEM0001|2026-12-19T10:00:00|200000\n");
        writeFile(f.ticketPath, "TKT0001|INV0001|SHW0001|A1|REGULAR|100000|10000|90000|0|SOLD||||0\n");
        require(f.invoices.load(), "invoice load");
        require(f.tickets.load(), "ticket load");
        Invoice* invoice = f.invoices.findById("INV0001");
        Ticket* ticket = f.tickets.findById("TKT0001");
        require(invoice && ticket && invoice->getTicketCount() == 1, "count");
        require(invoice->getTicketAt(0) == ticket && ticket->getInvoice() == invoice, "link hai chieu");
    });

    test("Load Ticket trung ghe SOLD phai rollback", [](Fixture& f) {
        writeFile(f.invoicePath, "INV0001|SHW0001|USR0001|MEM0001|2026-12-19T10:00:00|200000\n");
        writeFile(f.ticketPath,
                  "TKT0001|INV0001|SHW0001|A1|REGULAR|100000|10000|90000|0|SOLD||||0\n"
                  "TKT0002|INV0001|SHW0001|A1|REGULAR|100000|10000|90000|0|SOLD||||0\n");
        require(f.invoices.load(), "invoice load");
        bool threw = false;
        try { f.tickets.load(); } catch (const exception&) { threw = true; }
        require(threw, "phai nem exception");
        require(!f.tickets.isLoaded() && f.tickets.count() == 0, "clear sau loi");
        require(f.invoices.findById("INV0001")->getTicketCount() == 0, "khong de lai link");
    });

    test("Load Invoice sai tham chieu phai rollback", [](Fixture& f) {
        writeFile(f.invoicePath, "INV0001|SHW9999|USR0001|MEM0001|2026-12-19T10:00:00|200000\n");
        bool threw = false;
        try { f.invoices.load(); } catch (const exception&) { threw = true; }
        require(threw && !f.invoices.isLoaded() && f.invoices.count() == 0, "rollback invoice");
        require(f.staff.getInvoiceCount() == 0 && f.member.getInvoiceCount() == 0, "khong de lai link");
    });

    test("Load Ticket sai schema phai rollback", [](Fixture& f) {
        require(f.invoices.load(), "invoice load");
        writeFile(f.ticketPath, "TKT0001|INV0001|SHW0001\n");
        bool threw = false;
        try { f.tickets.load(); } catch (const exception&) { threw = true; }
        require(threw && !f.tickets.isLoaded() && f.tickets.count() == 0, "rollback parse");
    });

    test("Khong load Ticket khi Invoice chua load", [](Fixture& f) {
        require(!f.tickets.load(), "phai tu choi");
        require(!f.tickets.isLoaded(), "giu unloaded");
    });

    cout << "\n===== SUMMARY =====\nPassed: " << passed
         << "\nFailed: " << failed
         << "\nTotal: " << passed + failed << '\n';
    return failed == 0 ? 0 : 1;
}
