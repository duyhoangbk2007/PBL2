#include "CatalogView.h"
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <exception>
#include <limits>

void CatalogUi::run(QLabel* status, const std::function<void()>& action)
{
    try {
        action();
    } catch (const std::exception& error) {
        status->setText(QString("Không thực hiện được thao tác. %1").arg(QString::fromUtf8(error.what())));
    }
}

bool CatalogUi::confirmForm(QDialog& dialog, QFormLayout& form, bool creating)
{
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    if (creating) {
        // TODO: BACKEND INTEGRATION — persist the issued ID together with the new entity.
        form.addRow(new QLabel("Service đã cấp mã. Đang chờ tích hợp lưu bản ghi mới."));
        buttons.button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    form.addRow(&buttons);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    return dialog.exec() == QDialog::Accepted && !creating;
}

void CatalogUi::result(QLabel* status, bool changed)
{
    status->setText(changed ? "Đã cập nhật trong bộ nhớ. Chưa ghi xuống tệp."
                            : "Service từ chối thao tác. Kiểm tra giá trị và dữ liệu liên kết.");
}

QSpinBox* CatalogUi::number(QFormLayout& form, const QString& caption, int value, int minimum)
{
    auto* field = new QSpinBox;
    field->setRange(minimum, std::numeric_limits<int>::max());
    field->setValue(value);
    form.addRow(caption, field);
    return field;
}

CatalogView::CatalogView(const QStringList& columns)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    toolbar = new QHBoxLayout;
    search = new QLineEdit;
    search->setPlaceholderText("Tìm theo mã hoặc tên…");
    search->setClearButtonEnabled(true);
    toolbar->addWidget(search, 1);
    layout->addLayout(toolbar);
    table = new QTableWidget(0, columns.size());
    table->setHorizontalHeaderLabels(columns);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(43);
    table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto* panel = new QFrame;
    panel->setObjectName("panel");
    auto* body = new QVBoxLayout(panel);
    body->setContentsMargins(10, 10, 10, 10);
    table->setFrameShape(QFrame::NoFrame);
    body->addWidget(table);
    layout->addWidget(panel, 1);
    status = new QLabel;
    status->setTextFormat(Qt::PlainText);
    status->setWordWrap(true);
    layout->addWidget(status);
    connect(search, &QLineEdit::textChanged, this, &CatalogView::filter);
}

void CatalogView::append(const QStringList& values, const QString& link, const QString& id)
{
    const int row = table->rowCount();
    table->insertRow(row);
    for (int i = 0; i < values.size(); ++i) {
        auto* cell = new QTableWidgetItem(values.at(i));
        cell->setToolTip(values.at(i));
        table->setItem(row, i, cell);
    }
    if (!link.isEmpty()) {
        auto* button = new QPushButton(link);
        table->setCellWidget(row, values.size(), button);
        connect(button, &QPushButton::clicked, this, [this, id] { emit openRelated(id); });
    }
}

void CatalogView::filter()
{
    const QString query = search->text().trimmed();
    for (int i = 0; i < table->rowCount(); ++i) {
        bool match = table->item(i, 0)->text().contains(query, Qt::CaseInsensitive)
                  || table->item(i, 1)->text().contains(query, Qt::CaseInsensitive);
        table->setRowHidden(i, !match);
    }
}

QString CatalogView::selectedId() const
{
    const int row = table->currentRow();
    if (row < 0 || table->isRowHidden(row)) {
        return {};
    }
    return table->item(row, 0)->text();
}

void CatalogView::action(const QString& caption, bool allowed, const std::function<void()>& handler)
{
    if (allowed) {
        auto* button = new QPushButton(caption);
        toolbar->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, handler] {
            CatalogUi::run(status, handler);
        });
    }
}

void CatalogView::pending(const QString& caption, bool allowed, const QString& reason)
{
    // TODO: BACKEND INTEGRATION — enable when the corresponding service API exists.
    if (allowed) {
        auto* button = new QPushButton(caption);
        button->setToolTip(reason);
        button->setEnabled(false);
        toolbar->addWidget(button);
    }
}
