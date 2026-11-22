#include "InvoiceHistoryView.h"
#include "HistoryWidgets.h"
#include "PricingManagementView.h"
#include "../business/AuthService.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

InvoiceHistoryView::InvoiceHistoryView(InvoiceService& service, AuthService& auth)
    : auth(auth), service(service), dates(new HistoryUi::Dates), code(new QLineEdit), staff(new QLineEdit),
      movie(new QLineEdit), show(new QLineEdit), invoices(HistoryUi::table({"Mã hóa đơn", "Thời điểm lập",
      "Nhân viên", "Phim", "Suất chiếu", "Phòng", "Thành viên", "Số vé", "Tổng ban đầu", "Đã hoàn",
      "Còn lại", "Trạng thái"})), details(new QPushButton("Xem chi tiết")), status(HistoryUi::label({}, "demoNotice")),
      preview(new QCheckBox("Xem dữ liệu DEMO"))
{
    setObjectName("invoiceHistory");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* filters = HistoryUi::panel(layout, {});
    auto* fields = new QGridLayout;
    int col = 0;
    for (auto* field : {code, staff, movie, show}) {
        const QString name = QStringList{"Mã hóa đơn", "Nhân viên lập", "Phim", "Suất chiếu"}.at(col);
        fields->addWidget(new QLabel(name), 0, col);
        fields->addWidget(field, 1, col++);
        field->setAccessibleName(name);
        field->setPlaceholderText(field == code ? "Nhập mã hóa đơn" : "Tất cả");
        connect(field, &QLineEdit::returnPressed, this, &InvoiceHistoryView::search);
    }
    code->setObjectName("invoiceCode");
    filters->addLayout(fields);
    auto* actions = new QHBoxLayout;
    actions->addWidget(dates);
    actions->addStretch();
    auto* apply = new QPushButton("Tra cứu");
    auto* clear = new QPushButton("Xóa bộ lọc");
    actions->addWidget(apply);
    actions->addWidget(clear);
    filters->addLayout(actions);
    const bool all = auth.hasPermission("INVOICE_VIEW_ALL");
    staff->setEnabled(all);
    if (!all) {
        staff->setPlaceholderText("Tài khoản hiện tại");
        filters->addWidget(HistoryUi::label("Hóa đơn của bạn trong ngày. Để tra cứu hóa đơn cũ, nhập đúng mã hóa đơn."));
    }
    preview->setVisible(all && auth.isAdmin());
    filters->addWidget(preview);
    layout->addWidget(status);
    auto* list = HistoryUi::panel(layout, "Danh sách hóa đơn");
    layout->setStretchFactor(list->parentWidget(), 1);
    list->addWidget(invoices, 1);
    list->addWidget(details, 0, Qt::AlignRight);
    connect(apply, &QPushButton::clicked, this, &InvoiceHistoryView::search);
    connect(clear, &QPushButton::clicked, this, [this] {
        for (auto* field : {code, staff, movie, show}) { field->clear(); }
        dates->setRange();
        search();
    });
    connect(preview, &QCheckBox::toggled, this, &InvoiceHistoryView::search);
    connect(invoices, &QTableWidget::itemSelectionChanged, this, [this] {
        details->setEnabled(!invoices->selectedItems().isEmpty());
    });
    connect(details, &QPushButton::clicked, this, &InvoiceHistoryView::detail);
    connect(invoices, &QTableWidget::cellDoubleClicked, this, &InvoiceHistoryView::detail);
    search();
}

void InvoiceHistoryView::search()
{
    result = SalesHistory::history(service, auth, {dates->period(), code->text(), staff->text(), movie->text(), show->text()},
                                   preview->isChecked());
    invoices->setRowCount(0);
    details->setEnabled(false);
    for (const auto& i : result.invoices) {
        HistoryUi::append(invoices, {i.id, HistoryUi::timestamp(i.created), i.staff, i.movie, i.show, i.room,
            i.member, QString::number(i.tickets.size()), formatVnd(i.original), formatVnd(i.refunded),
            formatVnd(i.remaining), i.status});
    }
    HistoryUi::showStatus(status, result.status, result.invoices.isEmpty());
}

void InvoiceHistoryView::detail()
{
    const int row = invoices->currentRow();
    if (row < 0 || row >= result.invoices.size()) { return; }
    const auto& i = result.invoices[row];
    // Re-query by ID so a future detail service can enforce access at open time.
    const auto fresh = SalesHistory::history(service, auth, {{}, i.id, {}, {}, {}}, preview->isChecked());
    if (fresh.invoices.size() != 1 || fresh.invoices.first().id != i.id) { search(); return; }
    const auto& invoice = fresh.invoices.first();
    QDialog dialog(this);
    dialog.setWindowTitle("Chi tiết hóa đơn " + invoice.id);
    auto* body = new QVBoxLayout(&dialog);
    if (fresh.status.state == SalesHistory::State::Preview) {
        body->addWidget(HistoryUi::label("DEMO · Chi tiết hóa đơn minh họa", "demoNotice"));
    }
    body->addWidget(HistoryUi::label(HistoryUi::summary(invoice)));
    auto* lines = HistoryUi::table({"Mã vé", "Ghế", "Loại ghế", "Loại vé", "Giá gốc", "Giảm", "Thực trả",
                                    "Trạng thái", "Hủy lúc", "Người hủy", "Lý do", "Tiền hoàn"});
    for (const auto& t : invoice.tickets) {
        HistoryUi::append(lines, {t.id, t.seat, t.seatType, t.ticketType, formatVnd(t.base), formatVnd(t.discount),
            formatVnd(t.paid), t.status, t.cancelledAt, t.cancelledBy, t.reason, formatVnd(t.refunded)});
    }
    body->addWidget(lines, 1);
    body->addWidget(HistoryUi::label(QString("Tổng ban đầu %1     Đã hoàn %2     Còn lại %3")
        .arg(formatVnd(invoice.original), formatVnd(invoice.refunded), formatVnd(invoice.remaining))));
    auto* close = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(close, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    body->addWidget(close);
    dialog.resize(870, 370);
    dialog.exec();
}
