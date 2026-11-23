#include "../src/gui/ApplicationContext.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QDate>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void writeFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    require(file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
            file.write(content) == content.size(), "cannot write fixture");
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    require(file.open(QIODevice::ReadOnly), "cannot read fixture");
    return file.readAll();
}

void prepare(const QDir& dir)
{
    const QByteArray future = QDate::currentDate().addDays(5).toString(Qt::ISODate).toUtf8();
    writeFile(dir.filePath("users.txt"), "USR0001|admin|123456|Admin|ADMIN|1\n");
    writeFile(dir.filePath("role_permissions.txt"),
              "ADMIN|MOVIE_VIEW\nADMIN|MOVIE_ADD\nADMIN|STAFF_VIEW\nADMIN|STAFF_ADD\n"
              "ADMIN|MEMBER_VIEW\nADMIN|MEMBER_ADD\nADMIN|PRICING_VIEW\nADMIN|PRICING_UPDATE\n"
              "ADMIN|TICKET_SELL\nADMIN|TICKET_CANCEL\nADMIN|INVOICE_VIEW_ALL\n");
    writeFile(dir.filePath("movies.txt"), "MOV0001|Film|Drama|90|0|2020-01-01|2099-12-31|1\n");
    writeFile(dir.filePath("rooms.txt"), "ROM0001|Room|2|2|1\n");
    writeFile(dir.filePath("seats.txt"),
              "ROM0001|A01|1|1|STANDARD\nROM0001|A02|1|2|STANDARD\n");
    writeFile(dir.filePath("showtimes.txt"),
              "SHW0001|MOV0001|ROM0001|" + future + "T10:00:00|" + future + "T12:00:00|SCHEDULED\n");
    writeFile(dir.filePath("members.txt"), "MEM0001|Member|0912345678|0\n");
    writeFile(dir.filePath("pricing.txt"),
              "STANDARD|75000\nVIP|100000\nCOUPLE|160000\nSTUDENT_DISCOUNT|10\nMEMBER_DISCOUNT|0\n");
    writeFile(dir.filePath("invoices.txt"), "");
    writeFile(dir.filePath("tickets.txt"), "");
}

void testSaveAndRestart()
{
    QTemporaryDir temp;
    require(temp.isValid(), "temporary directory");
    const QDir dir(temp.path());
    prepare(dir);
    {
        ApplicationContext ctx(temp.path());
        require(ctx.load().isEmpty() && ctx.catalogError.isEmpty() &&
                ctx.transactionError.isEmpty() && ctx.memberError.isEmpty() &&
                ctx.pricingError.isEmpty(), "load failed");
        require(ctx.auth.login("admin", "123456"), "admin login failed");
        Movie movie("", "New Film", "Drama", 100, 0, "2020-01-01", "2099-12-31", true);
        User staff("", "newstaff", "123456", "New Staff", UserRole::STAFF, true);
        Member member("", "New Member", "0999999999", 0);
        require(ctx.movies.addMovie(&movie) && ctx.staff.addStaff(&staff) &&
                ctx.members.addMember(&member), "catalog update failed");
        require(ctx.pricing.updateSeatPrice(SeatType::STANDARD, 80000), "pricing update failed");
        std::string firstInvoice;
        require(ctx.booking.createBooking("SHW0001", {{"A01", TicketType::REGULAR}},
                                          "MEM0001", true, true, firstInvoice) &&
                firstInvoice == "INV0001", "first booking failed");
        require(ctx.booking.cancelTicket("TKT0001", "Khach doi lich", true), "cancel failed");
        std::string secondInvoice;
        require(ctx.booking.createBooking("SHW0001", {{"A02", TicketType::REGULAR}},
                                          "MEM0001", true, true, secondInvoice) &&
                secondInvoice == "INV0002", "second booking failed");
        require(readFile(dir.filePath("tickets.txt")).isEmpty() &&
                readFile(dir.filePath("invoices.txt")).isEmpty() &&
                readFile(dir.filePath("pricing.txt")).contains("STANDARD|75000"),
                "operation wrote TXT before saveAll");
        require(ctx.saveAll().isEmpty(), "saveAll failed");
    }
    {
        ApplicationContext restarted(temp.path());
        require(restarted.load().isEmpty() && restarted.catalogError.isEmpty() &&
                restarted.transactionError.isEmpty() && restarted.memberError.isEmpty() &&
                restarted.pricingError.isEmpty(), "restart load failed");
        require(restarted.auth.login("admin", "123456"), "restart login failed");
        require(restarted.movies.findMovieById("MOV0002") != nullptr &&
                restarted.staff.findStaffByUsername("newstaff") != nullptr &&
                restarted.members.findMemberById("MEM0002") != nullptr,
                "catalog, staff or member missing after restart");
        require(restarted.pricing.getBasePrice(SeatType::STANDARD) == 80000 &&
                restarted.members.findMemberById("MEM0001")->getPoints() == 8,
                "pricing or points missing after restart");
        Invoice* first = restarted.invoices.findInvoiceById("INV0001");
        Invoice* second = restarted.invoices.findInvoiceById("INV0002");
        require(first && second && first->getTicketCount() == 1 &&
                second->getTicketCount() == 1 &&
                first->getTicketAt(0)->getStatus() == TicketStatus::CANCELLED &&
                second->getTicketAt(0)->getStatus() == TicketStatus::SOLD,
                "invoice or ticket state missing after restart");
    }
}

void testUnloadedAndPartialFailure()
{
    QTemporaryDir temp;
    require(temp.isValid(), "temporary directory");
    const QDir dir(temp.path());
    prepare(dir);
    writeFile(dir.filePath("pricing.txt"), "BROKEN|1\n");
    {
        ApplicationContext ctx(temp.path());
        require(ctx.load().isEmpty() && !ctx.pricingError.isEmpty(), "invalid pricing was loaded");
        const QByteArray before = readFile(dir.filePath("users.txt"));
        const QString result = ctx.saveAll();
        require(result.contains("pricing.txt") && result.contains("Chua ghi") &&
                readFile(dir.filePath("pricing.txt")) == "BROKEN|1\n" &&
                readFile(dir.filePath("users.txt")) == before,
                "unloaded repository was saved");
    }
    prepare(dir);
    {
        ApplicationContext ctx(temp.path());
        require(ctx.load().isEmpty() && ctx.pricingError.isEmpty(), "load for partial failure failed");
        require(ctx.auth.login("admin", "123456") &&
                ctx.pricing.updateSeatPrice(SeatType::STANDARD, 81000), "RAM update failed");
        require(QFile::remove(dir.filePath("tickets.txt")) &&
                QDir().mkdir(dir.filePath("tickets.txt")), "prepare write failure failed");
        const QString result = ctx.saveAll();
        require(result.contains("tickets.txt") && result.contains("Da ghi") &&
                readFile(dir.filePath("pricing.txt")).contains("STANDARD|81000"),
                "partial save not reported");
        require(QDir().rmdir(dir.filePath("tickets.txt")), "remove simulated failure failed");
        require(ctx.saveAll().isEmpty(), "retry save failed");
    }
}
}

int main()
{
    try {
        testSaveAndRestart();
        testUnloadedAndPartialFailure();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
