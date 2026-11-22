#pragma once
#include "SalesHistoryAdapter.h"
#include <QWidget>
class AuthService;
class BookingService;
class InvoiceService;
class QCheckBox;
class QLabel;
class QLineEdit;
class QTableWidget;

class TicketCancellationView : public QWidget
{
public:
    explicit TicketCancellationView(BookingService& booking, InvoiceService& invoices, AuthService& auth);
private:
    void lookup();
    void confirm(const SalesHistory::Invoice& invoice, const SalesHistory::TicketLine& ticket);
    AuthService& auth;
    BookingService& booking;
    InvoiceService& invoices;
    bool storageError = false;
    QCheckBox* preview;
    QLineEdit* query;
    QLabel *status, *invoiceInfo;
    QTableWidget* tickets;
};
