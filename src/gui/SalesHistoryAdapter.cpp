#include "SalesHistoryAdapter.h"
#include "../business/AuthService.h"

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
}
