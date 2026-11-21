#pragma once
#include "SalesHistoryAdapter.h"
#include <QWidget>
#include <QStringList>

class QDateEdit;
class QLabel;
class QTableWidget;
class QVBoxLayout;

namespace HistoryUi {
QLabel* label(const QString& text, const QString& style = {});
QVBoxLayout* panel(QVBoxLayout* parent, const QString& heading);
QTableWidget* table(const QStringList& columns);
int append(QTableWidget* table, const QStringList& values);
QString timestamp(const QDateTime& time);
QString summary(const SalesHistory::Invoice& invoice);
void showStatus(QLabel* label, const SalesHistory::Status& status, bool empty);
class Dates : public QWidget
{
public:
    explicit Dates(QWidget* parent = nullptr);
    SalesHistory::Period period() const;
    void setRange(QDate first = {}, QDate last = {});
private:
    QDateEdit* from;
    QDateEdit* to;
};
}
