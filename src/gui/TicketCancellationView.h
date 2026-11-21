#pragma once
#include "SalesHistoryAdapter.h"
#include <QWidget>
class AuthService;
class QCheckBox;
class QLabel;
class QLineEdit;
class QTableWidget;

class TicketCancellationView : public QWidget
{
public:
    explicit TicketCancellationView(AuthService& auth);
private:
    void lookup();
    void confirm(const SalesHistory::Invoice& invoice, const SalesHistory::TicketLine& ticket);
    AuthService& auth;
    QCheckBox* preview;
    QLineEdit* query;
    QLabel *status, *invoiceInfo;
    QTableWidget* tickets;
};
