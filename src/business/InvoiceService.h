#ifndef INVOICE_SERVICE_H
#define INVOICE_SERVICE_H

#include <string>
#include <vector>

#include "../data_access/InvoiceRepository.h"
#include "../data_access/TicketRepository.h"
#include "AuthService.h"

enum class InvoiceStatus
{
    ACTIVE,
    PARTIALLY_CANCELLED,
    CANCELLED
};

class InvoiceService
{
private:
    InvoiceRepository& invoiceRepo;
    TicketRepository& ticketRepo;
    AuthService& authService;

    const User& requireViewPermission() const;
    bool canViewInvoice(const Invoice* invoice) const;
    Invoice* requireInvoice(const std::string& invoiceId);
    std::vector<Ticket*> getValidTickets(Invoice* invoice);

public:
    InvoiceService(InvoiceRepository& invoiceRepo,
                   TicketRepository& ticketRepo,
                   AuthService& authService);

    Invoice* findInvoiceById(const std::string& invoiceId);

    std::vector<Invoice*> getInvoiceHistory(const std::string& from,
                                            const std::string& to,
                                            const std::string& staffId = "",
                                            const std::string& memberId = "");

    InvoiceStatus getInvoiceStatus(const std::string& invoiceId);
    double getRefundAmount(const std::string& invoiceId);
    double getRemainingAmount(const std::string& invoiceId);
};

#endif
