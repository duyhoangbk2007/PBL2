#pragma once
#include "SalesHistoryAdapter.h"
#include <QWidget>
class AuthService;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
namespace HistoryUi { class Dates; }

class InvoiceHistoryView : public QWidget
{
public:
    explicit InvoiceHistoryView(AuthService& auth);
private:
    void search();
    void detail();
    AuthService& auth;
    HistoryUi::Dates* dates;
    QLineEdit *code, *staff, *movie, *show;
    QTableWidget* invoices;
    QPushButton* details;
    QLabel* status;
    QCheckBox* preview;
    SalesHistory::InvoiceResult result;
};
