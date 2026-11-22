#include "TicketCancellationView.h"
#include "HistoryWidgets.h"
#include "PricingManagementView.h"
#include "../business/AuthService.h"
#include "../business/BookingService.h"
#include "../business/InvoiceService.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <exception>

TicketCancellationView::TicketCancellationView(BookingService& booking, InvoiceService& invoices, AuthService& auth)
    : auth(auth), booking(booking), invoices(invoices), preview(new QCheckBox("Xem dữ liệu DEMO")), query(new QLineEdit),
      status(HistoryUi::label({}, "demoNotice")), invoiceInfo(HistoryUi::label({})),
      tickets(HistoryUi::table({"Mã vé", "Ghế", "Suất chiếu", "Bắt đầu", "Thực trả", "Trạng thái", "Thao tác"}))
{
    setObjectName("ticketCancellation");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* filters = HistoryUi::panel(layout, {});
    auto* row = new QHBoxLayout;
    query->setAccessibleName("Mã vé hoặc mã hóa đơn");
    query->setPlaceholderText("Tìm theo mã vé hoặc mã hóa đơn…");
    row->addWidget(query, 1);
    auto* clear = new QPushButton("Xóa tìm kiếm");
    row->addWidget(clear);
    filters->addLayout(row);
    filters->addWidget(preview);
    layout->addWidget(status);
    auto* list = HistoryUi::panel(layout, "Vé trong hóa đơn");
    layout->setStretchFactor(list->parentWidget(), 1);
    list->addWidget(invoiceInfo);
    tickets->horizontalHeader()->setStretchLastSection(false);
    tickets->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    list->addWidget(tickets, 1);
    list->addWidget(HistoryUi::label("Chỉ hủy từng vé SOLD trước giờ chiếu. Hóa đơn gốc được giữ lại.", "description"));
    connect(clear, &QPushButton::clicked, query, &QLineEdit::clear);
    connect(query, &QLineEdit::textChanged, this, &TicketCancellationView::lookup);
    connect(preview, &QCheckBox::toggled, this, &TicketCancellationView::lookup);
    lookup();
}

void TicketCancellationView::lookup()
{
    const auto result = SalesHistory::cancellationLookup(invoices, auth, query->text(), preview->isChecked());
    tickets->setRowCount(0);
    invoiceInfo->setText("Chưa chọn hóa đơn.");
    for (const auto& invoice : result.invoices) {
        invoiceInfo->setText(HistoryUi::summary(invoice) +
            "\nĐã hoàn: " + formatVnd(invoice.refunded) +
            "   Còn lại: " + formatVnd(invoice.remaining));
        for (const auto& ticket : invoice.tickets) {
            const int row = HistoryUi::append(tickets, {ticket.id, ticket.seat, invoice.show,
                HistoryUi::timestamp(invoice.starts), formatVnd(ticket.paid), ticket.status,
                ticket.canCancel ? QString() : QString("—")});
            if (ticket.canCancel && !storageError) {
                auto* action = new QPushButton("Hủy vé");
                action->setAccessibleName("Hủy vé " + ticket.id);
                tickets->setCellWidget(row, 6, action);
                connect(action, &QPushButton::clicked, this, [this, invoice, ticket] { confirm(invoice, ticket); });
            }
        }
    }
    tickets->resizeRowsToContents();
    HistoryUi::showStatus(status, result.status, result.invoices.isEmpty());
}

void TicketCancellationView::confirm(const SalesHistory::Invoice& invoice, const SalesHistory::TicketLine& ticket)
{
    if (!auth.isAdmin() || !auth.hasPermission("TICKET_CANCEL") || storageError) { return; }
    QDialog dialog(this);
    dialog.setWindowTitle("Hủy vé " + ticket.id);
    auto* form = new QVBoxLayout(&dialog);
    if (preview->isChecked()) {
        form->addWidget(HistoryUi::label("DEMO · Xem trước phiếu hủy vé", "demoNotice"));
    }
    form->addWidget(HistoryUi::label(QString("%1  ·  Ghế %2  ·  %3")
        .arg(invoice.id, ticket.seat, HistoryUi::timestamp(invoice.starts))));
    if (preview->isChecked()) {
        form->addWidget(HistoryUi::label("Số tiền dự kiến hoàn  " + formatVnd(ticket.proposedRefund)));
    } else {
        form->addWidget(HistoryUi::label("Báo giá hoàn đang BLOCKED: thiếu API Service.", "description"));
    }
    form->addWidget(new QLabel("Lý do hủy"));
    auto* reason = new QPlainTextEdit;
    reason->setAccessibleName("Lý do hủy");
    reason->setPlaceholderText("Nhập lý do hủy vé");
    form->addWidget(reason);
    auto* refund = new QCheckBox("Đã xác nhận hoàn tiền tại quầy");
    form->addWidget(refund);
    auto* message = HistoryUi::label({}, "description");
    form->addWidget(message);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    auto* submit = buttons->addButton("Xác nhận hủy vé", QDialogButtonBox::AcceptRole);
    submit->setEnabled(false);
    const auto update = [=] {
        submit->setEnabled(!storageError && !preview->isChecked() &&
                           !reason->toPlainText().trimmed().isEmpty() && refund->isChecked());
    };
    connect(reason, &QPlainTextEdit::textChanged, this, update);
    connect(refund, &QCheckBox::toggled, this, update);
    connect(submit, &QPushButton::clicked, this, [this, &dialog, ticket, reason, refund, submit, message, update] {
        submit->setEnabled(false);
        try {
            if (!booking.cancelTicket(ticket.id.toStdString(), reason->toPlainText().trimmed().toStdString(),
                                      refund->isChecked())) {
                message->setText("Không thể hủy vé. Kiểm tra mã vé, trạng thái và điều kiện giao dịch.");
                update();
                return;
            }
            dialog.accept();
            lookup();
        } catch (const std::exception& error) {
            storageError = true;
            message->setText("Lỗi giao dịch: " + QString::fromUtf8(error.what()));
            status->setText(message->text());
            submit->setEnabled(false);
        }
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addWidget(buttons);
    dialog.resize(470, 310);
    dialog.exec();
}
