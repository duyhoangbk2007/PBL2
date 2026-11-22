#include "InvoiceService.h"
#include "../utils/DateTime.h"
#include "../utils/Validator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

InvoiceService::InvoiceService(InvoiceRepository& invoiceRepo,
                               TicketRepository& ticketRepo,
                               AuthService& authService)
    : invoiceRepo(invoiceRepo),
      ticketRepo(ticketRepo),
      authService(authService)
{
}

// Trả về quyền hiện tại tùy vào admin hoặc staff
const User& InvoiceService::requireViewPermission() const
{
    const User& user = authService.requireLogin();
    if (user.getRole() == UserRole::ADMIN)
    {
        authService.requirePermission("INVOICE_VIEW_ALL");
    }
    else if (user.getRole() == UserRole::STAFF)
    {
        authService.requirePermission("INVOICE_VIEW_OWN");
    }
    else
    {
        throw runtime_error("Vai tro khong duoc xem hoa don");
    }
    return user;
}

// admin được xem mọi hóa đơn, staff chỉ xem hóa đơn có ID của mình
bool InvoiceService::canViewInvoice(const Invoice* invoice) const
{
    const User* user = authService.getCurrentUser();
    if (invoice == nullptr || user == nullptr)
    {
        return false;
    }
    if (user->getRole() == UserRole::ADMIN)
    {
        return true;
    }
    return user->getRole() == UserRole::STAFF &&
           invoice->getStaffId() == user->getUserId();
}

Invoice* InvoiceService::findInvoiceById(const string& invoiceId)
{
    requireViewPermission();
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
    Invoice* invoice = invoiceRepo.findById(invoiceId);
    if (invoice == nullptr)
    {
        return nullptr;
    }
    if (!canViewInvoice(invoice))
    {
        throw runtime_error("Khong duoc xem hoa don cua nhan vien khac");
    }
    return invoice;
}

Invoice* InvoiceService::requireInvoice(const string& invoiceId)
{
    Invoice* invoice = findInvoiceById(invoiceId);
    if (invoice == nullptr)
    {
        throw invalid_argument("Khong tim thay hoa don: " + invoiceId);
    }
    return invoice;
}

bool compareInvoices(const Invoice* first, const Invoice* second)
{
    if (first->getCreatedAt() != second->getCreatedAt())
    {
        return first->getCreatedAt() > second->getCreatedAt();
    }
    return first->getInvoiceId() < second->getInvoiceId();
}

vector<Invoice*> InvoiceService::getInvoiceHistory(const string& from, const string& to, const string& staffId, const string& memberId)
{
    const User& user = requireViewPermission();
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
    if (!Validator::isValidDateTime(from) ||
        !Validator::isValidDateTime(to) ||
        !DateTime::isBefore(from, to))
    {
        throw invalid_argument("Khoang thoi gian khong hop le");
    }
    // staff chỉ được coi lịch sử trong ngày
    if (user.getRole() == UserRole::STAFF)
    {
        string todayStart = DateTime::getCurrentDateTime().substr(0, 10) + "T00:00:00";
        string tomorrowStart = DateTime::addMinutes(todayStart, 24 * 60);
        if (from < todayStart || to > tomorrowStart)
        {
            throw invalid_argument("Staff chi duoc liet ke hoa don trong ngay hien tai");
        }
        if (!staffId.empty() && staffId != user.getUserId())
        {
            throw runtime_error("Staff khong duoc loc hoa don cua nhan vien khac");
        }
    }
    
    vector<Invoice*> result;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        if (invoice == nullptr || !Validator::isValidDateTime(invoice->getCreatedAt()))
        {
            throw runtime_error("Du lieu hoa don khong nhat quan");
        }
        if (invoice->getCreatedAt() < from || invoice->getCreatedAt() >= to ||
            !canViewInvoice(invoice) ||
            (!staffId.empty() && invoice->getStaffId() != staffId) ||
            (!memberId.empty() && invoice->getMemberId() != memberId))
        {
            continue;
        }
        result.push_back(invoice);
    }
    sort(result.begin(), result.end(), compareInvoices);
    return result;
}

vector<Ticket*> InvoiceService::getValidTickets(Invoice* invoice)
{
    if (!ticketRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu ve");
    }
    vector<Ticket*> tickets = ticketRepo.getByInvoice(invoice->getInvoiceId());
    if (tickets.empty() || invoice->getTicketCount() != static_cast<int>(tickets.size()))
    {
        throw runtime_error("Hoa don khong co ve hoac lien ket ve khong nhat quan");
    }
    double paidTotal = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket == nullptr || ticket->getInvoice() != invoice ||
            ticket->getInvoiceId() != invoice->getInvoiceId() ||
            ticket->getShowtimeId() != invoice->getShowtimeId() ||
            !isfinite(ticket->getPaidAmount()) || ticket->getPaidAmount() < 0 ||
            !isfinite(ticket->getRefundAmount()) || ticket->getRefundAmount() < 0)
        {
            throw runtime_error("Du lieu ve khong nhat quan");
        }
        // kiểm tra liên kết của mỗi vé với invoice
        bool linked = false;
        for (int i = 0; i < invoice->getTicketCount(); i++)
        {
            if (invoice->getTicketAt(i) == ticket)
            {
                linked = true;
                break;
            }
        }
        if (!linked)
        {
            throw runtime_error("Ve khong nam trong danh sach cua hoa don");
        }
        // nếu vé SOLD thì không được có tiền hoàn trả
        if (ticket->getStatus() == TicketStatus::SOLD)
        {
            if (ticket->getRefundAmount() != 0)
            {
                throw runtime_error("Ve dang ban co tien hoan");
            }
        }
        else if (ticket->getStatus() == TicketStatus::CANCELLED)
        {
            if (fabs(ticket->getRefundAmount() - ticket->getPaidAmount()) > 0.000001)
            {
                throw runtime_error("Tien hoan ve khong nhat quan");
            }
        }
        else
        {
            throw runtime_error("Trang thai ve khong hop le");
        }
        paidTotal += ticket->getPaidAmount();
    }
    if (!isfinite(invoice->getTotalAmount()) || invoice->getTotalAmount() < 0 ||
        !isfinite(paidTotal) ||
        fabs(paidTotal - invoice->getTotalAmount()) > 0.00001)
    {
        throw runtime_error("Tong tien hoa don khong khop voi cac ve");
    }
    return tickets;
}

InvoiceStatus InvoiceService::getInvoiceStatus(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    vector<Ticket*> tickets = getValidTickets(invoice);
    int soldCount = 0;
    int cancelledCount = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket->getStatus() == TicketStatus::SOLD)
        {
            soldCount++;
        }
        else
        {
            cancelledCount++;
        }
    }
    if (cancelledCount == 0)
    {
        return InvoiceStatus::ACTIVE;
    }
    if (soldCount == 0)
    {
        return InvoiceStatus::CANCELLED;
    }
    return InvoiceStatus::PARTIALLY_CANCELLED;
}

double InvoiceService::getRefundAmount(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    vector<Ticket*> tickets = getValidTickets(invoice);
    double refund = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket->getStatus() == TicketStatus::CANCELLED)
        {
            refund += ticket->getRefundAmount();
        }
    }
    if (!isfinite(refund) || refund > invoice->getTotalAmount() + 0.00001)
    {
        throw runtime_error("Tong tien hoan vuot tien hoa don");
    }
    return refund;
}

double InvoiceService::getRemainingAmount(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    double refund = getRefundAmount(invoiceId);
    double remaining = invoice->getTotalAmount() - refund;
    if (!isfinite(remaining) || remaining < -0.00001)
    {
        throw runtime_error("Tien con lai cua hoa don khong hop le");
    }
    if (remaining < 0)
    {
        remaining = 0;
    }
    return remaining;
}
