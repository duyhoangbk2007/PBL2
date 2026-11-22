#include "HistoryWidgets.h"
#include "PricingManagementView.h"
#include <QDateEdit>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace HistoryUi {
QLabel* label(const QString& text, const QString& style)
{
    auto* widget = new QLabel(text);
    widget->setTextFormat(Qt::PlainText);
    widget->setWordWrap(true);
    widget->setObjectName(style);
    return widget;
}

QVBoxLayout* panel(QVBoxLayout* parent, const QString& heading)
{
    auto* frame = new QFrame;
    frame->setObjectName("panel");
    parent->addWidget(frame);
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(14, 10, 14, 12);
    if (!heading.isEmpty()) {
        layout->addWidget(label(heading, "panelHeading"));
    }
    return layout;
}

QTableWidget* table(const QStringList& columns)
{
    auto* widget = new QTableWidget(0, columns.size());
    widget->setHorizontalHeaderLabels(columns);
    widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    widget->setSelectionMode(QAbstractItemView::SingleSelection);
    widget->setShowGrid(false);
    widget->verticalHeader()->hide();
    widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    widget->horizontalHeader()->setStretchLastSection(true);
    return widget;
}

int append(QTableWidget* table, const QStringList& values)
{
    const int row = table->rowCount();
    table->insertRow(row);
    for (int col = 0; col < values.size(); ++col) {
        auto* item = new QTableWidgetItem(values[col]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setToolTip(values[col]);
        table->setItem(row, col, item);
    }
    return row;
}

QString timestamp(const QDateTime& time) { return time.toString("dd/MM/yyyy HH:mm"); }

QString summary(const SalesHistory::Invoice& invoice)
{
    return QString("%1  ·  %2  ·  %3\n%4  ·  %5  ·  %6  ·  %7\n%8  ·  %9")
        .arg(invoice.id, timestamp(invoice.created), invoice.staff, invoice.movie, invoice.show,
             timestamp(invoice.starts), invoice.room, invoice.member, invoice.status);
}

void showStatus(QLabel* banner, const SalesHistory::Status& status, bool empty)
{
    QString message = status.message;
    if (empty && (status.state == SalesHistory::State::Available || status.state == SalesHistory::State::Preview)) {
        message += "  Không có dữ liệu phù hợp.";
    }
    banner->setText(message);
    banner->setVisible(!message.isEmpty());
}

Dates::Dates(QWidget* parent) : QWidget(parent), from(new QDateEdit), to(new QDateEdit)
{
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    for (auto* edit : {from, to}) {
        const QString name = edit == from ? "Từ ngày" : "Đến ngày";
        edit->setObjectName(edit == from ? "fromDate" : "toDate");
        edit->setAccessibleName(name);
        edit->setCalendarPopup(true);
        edit->setDisplayFormat("dd/MM/yyyy");
        edit->setSpecialValueText("Mọi ngày");
        row->addWidget(new QLabel(name));
        row->addWidget(edit);
    }
    setRange();
}

void Dates::setRange(QDate first, QDate last)
{
    from->setDate(first.isValid() ? first : from->minimumDate());
    to->setDate(last.isValid() ? last : to->minimumDate());
}

SalesHistory::Period Dates::period() const
{
    return {from->date() == from->minimumDate() ? QDateTime() : from->date().startOfDay(),
            to->date() == to->minimumDate() ? QDateTime() : to->date().addDays(1).startOfDay()};
}
}
