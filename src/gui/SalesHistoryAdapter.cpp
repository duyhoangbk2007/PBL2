#include "SalesHistoryAdapter.h"
#include "../business/AuthService.h"
#include "../business/InvoiceService.h"
#include "../business/ReportService.h"
#include "../models/Invoice.h"
#include "../models/Ticket.h"
#include "../models/Showtime.h"
#include "../models/Movie.h"
#include "../models/Room.h"
#include "../models/Seat.h"
#include "../models/User.h"
#include "../models/Member.h"
#include <exception>
#include <stdexcept>
#include <limits>

namespace SalesHistory {
bool Period::valid() const
{
    return !from.isValid() || !until.isValid() || from < until;
}

bool Period::contains(const QDateTime& time) const
{
    return (!from.isValid() || time >= from) && (!until.isValid() || time < until);
}

namespace {
Status denied() { return {State::Error, "Tài khoản chưa được cấp quyền cho thao tác này."}; }
Status invalidDates() { return {State::Error, "Ngày kết thúc phải bằng hoặc sau ngày bắt đầu."}; }
Status demo() { return {State::Preview, "DEMO · Dữ liệu minh họa riêng, chỉ dùng để xem giao diện."}; }

Invoice exampleInvoice()
{
    const QDate today = QDate::currentDate();
    return {"DEMO-INV01", "NV mẫu 01", "Hành trình qua miền ký ức", "DEMO-S01", "Phòng 1",
            "Khách vãng lai", "PARTIALLY_CANCELLED", QDateTime(today, QTime(9, 15)),
            QDateTime(today.addDays(1), QTime(19, 30)), 145000, 45000, 100000,
            {{"DEMO-T01", "B02", "VIP", "REGULAR", "SOLD", 100000, 0, 100000,
              {}, {}, {}, 0, true, 100000},
             {"DEMO-T02", "A03", "STANDARD", "STUDENT", "CANCELLED", 50000, 5000, 45000,
              today.toString("dd/MM/yyyy") + " 10:05", "Admin mẫu", "Khách đổi lịch", 45000, false, 0}}};
}

bool matches(const QString& value, const QString& query)
{
    return value.contains(query.trimmed(), Qt::CaseInsensitive);
}

QList<Invoice> previewHistory()
{
    const Invoice mixed = exampleInvoice();
    const auto today = mixed.created.date();
    Invoice active{"DEMO-INV02", "NV mẫu 02", "Bình minh", "DEMO-S02", "Phòng 2", "Khách vãng lai",
                   "ACTIVE", today.addDays(-5).startOfDay(), QDateTime(today.addDays(-3), QTime(18, 0)),
                   70000, 0, 70000, {{"DEMO-T03", "A01", "STANDARD", "REGULAR", "SOLD", 70000, 0,
                   70000, {}, {}, {}, 0, false, 0}}};
    Invoice cancelled{"DEMO-INV03", "NV mẫu 01", "Qua đèo", "DEMO-S03", "Phòng 1", "Khách vãng lai",
                      "CANCELLED", today.addDays(-4).startOfDay(), QDateTime(today, QTime(18, 0)),
                      60000, 60000, 0, {{"DEMO-T04", "A05", "STANDARD", "REGULAR", "CANCELLED",
                      60000, 0, 60000, today.addDays(-1).toString("dd/MM/yyyy") + " 14:30", "Admin mẫu",
                      "Đổi suất chiếu", 60000, false, 0}}};
    return {mixed, active, cancelled};
}
}

InvoiceResult history(const AuthService& auth, const InvoiceFilter& filter, bool preview)
{
    const bool all = auth.hasPermission("INVOICE_VIEW_ALL");
    if (!all && !auth.hasPermission("INVOICE_VIEW_OWN")) {
        return {denied(), {}};
    }
    if (!filter.period.valid()) {
        return {invalidDates(), {}};
    }
    // TODO: BACKEND INTEGRATION — service must scope both list and detail to the session.
    if (!preview || !all || !auth.isAdmin()) {
        return {{State::NotImplemented, "Đang chờ dịch vụ tra cứu hóa đơn theo quyền người lập."}, {}};
    }
    InvoiceResult result{demo(), {}};
    for (const auto& invoice : previewHistory()) {
        if (filter.period.contains(invoice.created) && matches(invoice.id, filter.code)
            && matches(invoice.staff, filter.staff) && matches(invoice.movie, filter.movie)
            && matches(invoice.show, filter.show)) {
            result.invoices.append(invoice);
        }
    }
    return result;
}

InvoiceResult history(InvoiceService& service, const AuthService& auth, const InvoiceFilter& filter, bool preview)
{
    if (preview) { return history(auth, filter, true); }
    if (!filter.period.valid()) { return {invalidDates(), {}}; }

    InvoiceResult result{{State::Available, {}}, {}};
    try {
        const bool admin = auth.isAdmin();
        const QDate today = QDate::currentDate();
        const QDateTime from = filter.period.from.isValid() ? filter.period.from :
            admin ? QDate(1900, 1, 1).startOfDay() : today.startOfDay();
        const QDateTime until = filter.period.until.isValid() ? filter.period.until :
            admin ? QDate(9999, 12, 31).startOfDay() : today.addDays(1).startOfDay();
        if (from >= until) { return {invalidDates(), {}}; }

        std::vector<::Invoice*> source;
        const QString key = filter.code.trimmed();
        if (!admin && !key.isEmpty()) {
            ::Invoice* invoice = service.findInvoiceById(key.toStdString());
            if (invoice) { source.push_back(invoice); }
        } else {
            source = service.getInvoiceHistory(from.toString(Qt::ISODate).toStdString(),
                                               until.toString(Qt::ISODate).toStdString());
        }

        for (::Invoice* record : source) {
            const ::Showtime* show = record->getShowtime();
            const ::User* seller = record->getSeller();
            if (!show || !seller || !show->getMovie() || !show->getRoom()) {
                throw std::runtime_error("Lien ket hoa don khong day du");
            }
            Invoice item;
            item.id = QString::fromStdString(record->getInvoiceId());
            item.staff = QString::fromStdString(seller->getFullName());
            item.movie = QString::fromStdString(show->getMovie()->getTitle());
            item.show = QString::fromStdString(show->getShowtimeId());
            item.room = QString::fromStdString(show->getRoom()->getName());
            item.member = record->getMember() ?
                QString::fromStdString(record->getMember()->getFullName()) : "Khách vãng lai";
            item.created = QDateTime::fromString(QString::fromStdString(record->getCreatedAt()), Qt::ISODate);
            item.starts = QDateTime::fromString(QString::fromStdString(show->getStartAt()), Qt::ISODate);
            const auto state = service.getInvoiceStatus(record->getInvoiceId());
            item.status = state == InvoiceStatus::ACTIVE ? "ACTIVE" :
                state == InvoiceStatus::PARTIALLY_CANCELLED ? "PARTIALLY_CANCELLED" : "CANCELLED";
            item.original = record->getTotalAmount();
            item.refunded = service.getRefundAmount(record->getInvoiceId());
            item.remaining = service.getRemainingAmount(record->getInvoiceId());
            for (int index = 0; index < record->getTicketCount(); ++index) {
                ::Ticket* ticket = record->getTicketAt(index);
                if (!ticket || !ticket->getSeat()) {
                    throw std::runtime_error("Lien ket ve khong day du");
                }
                const SeatType kind = ticket->getSeat()->getSeatType();
                item.tickets.append({
                    QString::fromStdString(ticket->getTicketId()),
                    QString::fromStdString(ticket->getSeatId()),
                    kind == SeatType::STANDARD ? "STANDARD" : kind == SeatType::VIP ? "VIP" : "COUPLE",
                    ticket->getTicketType() == TicketType::REGULAR ? "REGULAR" : "STUDENT",
                    ticket->getStatus() == TicketStatus::SOLD ? "SOLD" : "CANCELLED",
                    ticket->getBasePrice(), ticket->getDiscountAmount(), ticket->getPaidAmount(),
                    QString::fromStdString(ticket->getCancelledAt()),
                    QString::fromStdString(ticket->getCancelledBy()),
                    QString::fromStdString(ticket->getCancelReason()), ticket->getRefundAmount(), false, 0
                });
            }
            if (filter.period.contains(item.created) && matches(item.id, filter.code) &&
                matches(item.staff, filter.staff) && matches(item.movie, filter.movie) &&
                matches(item.show, filter.show)) {
                result.invoices.append(item);
            }
        }
    } catch (const std::exception& error) {
        return {{State::Error, QString::fromUtf8(error.what())}, {}};
    }
    return result;
}

InvoiceResult cancellationLookup(const AuthService& auth, const QString& code, bool preview)
{
    if (!auth.isAdmin() || !auth.hasPermission("TICKET_CANCEL")) {
        return {denied(), {}};
    }
    // TODO: BACKEND INTEGRATION — request ticket eligibility and refund quote from the service.
    if (!preview) {
        return {{State::NotImplemented, "Đang chờ dịch vụ tra cứu, hủy vé và hoàn tiền."}, {}};
    }
    InvoiceResult result{demo(), {}};
    const auto key = code.trimmed();
    for (const auto& invoice : previewHistory()) {
        if (key.isEmpty() || invoice.id.compare(key, Qt::CaseInsensitive) == 0) {
            result.invoices.append(invoice);
            return result;
        }
        for (const auto& ticket : invoice.tickets) {
            if (ticket.id.compare(key, Qt::CaseInsensitive) == 0) {
                result.invoices.append(invoice);
                return result;
            }
        }
    }
    return result;
}

InvoiceResult cancellationLookup(InvoiceService& service, const AuthService& auth, const QString& code, bool preview)
{
    if (preview) { return cancellationLookup(auth, code, true); }
    if (!auth.isAdmin() || !auth.hasPermission("TICKET_CANCEL")) { return {denied(), {}}; }
    const QString key = code.trimmed();
    if (key.isEmpty()) {
        return {{State::Available, "Nhập mã hóa đơn để tra cứu. Tra cứu theo mã vé đang BLOCKED: thiếu API Service."}, {}};
    }
    if (!key.startsWith("INV", Qt::CaseInsensitive)) {
        return {{State::NotImplemented, "Tra cứu theo mã vé đang BLOCKED: thiếu API Service."}, {}};
    }
    try {
        const auto result = history(service, auth, {{}, key, {}, {}, {}}, false);
        if (result.status.state != State::Available) { return result; }
        if (result.invoices.size() != 1 || result.invoices.first().id.compare(key, Qt::CaseInsensitive) != 0) {
            return {{State::Available, "Không tìm thấy hóa đơn."}, {}};
        }
        InvoiceResult visible = result;
        visible.status.message = "Báo giá hoàn trước khi hủy đang BLOCKED: thiếu API Service. "
                                 "Service sẽ kiểm tra điều kiện hủy khi xác nhận.";
        for (auto& invoice : visible.invoices) {
            for (auto& ticket : invoice.tickets) {
                ticket.canCancel = ticket.status == "SOLD";
            }
        }
        return visible;
    } catch (const std::exception& error) {
        return {{State::Error, QString::fromUtf8(error.what())}, {}};
    }
}

Report report(const AuthService& auth, const Period& period, bool preview)
{
    if (!auth.hasPermission("REPORT_VIEW")) {
        return {denied()};
    }
    if (!period.valid()) {
        return {invalidDates()};
    }
    // TODO: BACKEND INTEGRATION — request cash flow and SOLD rankings, including tie breaks.
    if (!preview) {
        return {{State::NotImplemented, "Đang chờ dịch vụ thống kê doanh thu."}};
    }
    // Independent, precomputed display fixture. Yesterday includes a refund from an older sale.
    Report result{demo()};
    const auto today = QDate::currentDate();
    const bool first = period.contains(today.addDays(-1).startOfDay());
    const bool last = period.contains(today.startOfDay());
    if (first) {
        result.daily.append({today.addDays(-1).toString("dd/MM"), -60000});
    }
    if (last) {
        result.daily.append({today.toString("dd/MM"), 100000});
        result.movies.append({"Hành trình qua miền ký ức và những thành phố bên kia đại dương", 100000, 1});
    }
    result.sold = last ? 145000 : 0;
    result.refunded = first && last ? 105000 : first ? 60000 : last ? 45000 : 0;
    result.net = first && last ? 40000 : first ? -60000 : last ? 100000 : 0;
    result.issued = last ? 2 : 0;
    result.cancelled = int(first) + int(last);
    return result;
}

Report report(ReportService& service, const AuthService& auth, const Period& period, bool preview)
{
    if (preview) { return report(auth, period, true); }
    if (!auth.isAdmin() || !auth.hasPermission("REPORT_VIEW")) { return {denied()}; }
    if (!period.valid()) { return {invalidDates()}; }
    if (!period.from.isValid() || !period.until.isValid()) {
        return {{State::NotImplemented, "Chọn ngày bắt đầu và ngày kết thúc để tải báo cáo thật."}};
    }
    if (period.from.daysTo(period.until) > 366) {
        return {{State::NotImplemented, "NEEDS_VERIFICATION: biểu đồ theo ngày chỉ hỗ trợ kỳ tối đa 366 ngày."}};
    }
    try {
        const std::string from = period.from.toString(Qt::ISODate).toStdString();
        const std::string until = period.until.toString(Qt::ISODate).toStdString();
        Report result{{State::Available, "NEEDS_VERIFICATION: ReportService chưa cung cấp số vé theo từng phim; cột này hiển thị —."}};
        result.sold = service.getTotalSales(from, until);
        result.refunded = service.getTotalRefund(from, until);
        result.net = service.getActualRevenue(from, until);
        result.issued = service.getSoldTicketCount(from, until);
        result.cancelled = service.getCancelledTicketCount(from, until);
        for (QDateTime day = period.from; day < period.until; day = day.addDays(1)) {
            const QDateTime next = day.addDays(1) < period.until ? day.addDays(1) : period.until;
            result.daily.append({day.date().toString("dd/MM/yyyy"),
                service.getActualRevenue(day.toString(Qt::ISODate).toStdString(),
                                         next.toString(Qt::ISODate).toStdString())});
        }
        for (const Movie* movie : service.getTopMovies(from, until, std::numeric_limits<int>::max())) {
            if (!movie) { throw std::runtime_error("Du lieu phim bao cao khong hop le"); }
            result.movies.append({QString::fromStdString(movie->getTitle()),
                service.getMovieRevenue(movie->getMovieId(), from, until), -1});
        }
        return result;
    } catch (const std::exception& error) {
        return {{State::Error, QString::fromUtf8(error.what())}};
    }
}
}
