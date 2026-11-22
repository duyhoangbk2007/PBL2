#include "ReportView.h"
#include "HistoryWidgets.h"
#include "PricingManagementView.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>

namespace {
// A separate column for the bar keeps labels and amounts readable at every width.
class BarDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        QStyledItemDelegate::paint(painter, opt, index);
        const QRect area = opt.rect.adjusted(4, 6, -4, -6);
        const double ratio = index.data(Qt::UserRole).toDouble();
        const int zero = index.data(Qt::UserRole + 1).toBool() ? area.center().x() : area.left();
        const int end = zero + qRound(ratio * (area.right() - zero));
        painter->save();
        painter->setPen(QColor("#99a6bd"));
        painter->drawLine(zero, area.top(), zero, area.bottom());
        painter->fillRect(QRect(QPoint(qMin(zero, end), area.top()), QPoint(qMax(zero, end), area.bottom())),
                          ratio < 0 ? QColor("#be5864") : QColor("#536de3"));
        painter->restore();
    }
};

QTableWidget* chart(const QString& label, bool ranking)
{
    QStringList columns{label, "Biểu đồ", "Doanh thu"};
    if (ranking) { columns.append("Vé hợp lệ"); }
    auto* view = HistoryUi::table(columns);
    view->horizontalHeader()->setStretchLastSection(false);
    view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    view->setItemDelegateForColumn(1, new BarDelegate(view));
    view->setWordWrap(false);
    view->setTextElideMode(Qt::ElideRight);
    view->setMinimumHeight(100);
    return view;
}

void plot(QTableWidget* table, const QList<SalesHistory::ChartEntry>& rows, bool ranking)
{
    double scale = 1;
    bool signedAxis = false;
    for (const auto& item : rows) {
        scale = qMax(scale, std::abs(item.amount));
        signedAxis |= item.amount < 0;
    }
    for (const auto& item : rows) {
        QStringList values{item.label, {}, formatVnd(item.amount)};
        if (ranking) { values.append(QString::number(item.tickets)); }
        const int row = HistoryUi::append(table, values);
        table->item(row, 1)->setData(Qt::UserRole, item.amount / scale);
        table->item(row, 1)->setData(Qt::UserRole + 1, signedAxis);
    }
}
}

ReportView::ReportView(AuthService& auth)
    : auth(auth), dates(new HistoryUi::Dates), preview(new QCheckBox("Xem dữ liệu DEMO")),
      status(HistoryUi::label({}, "demoNotice")), daily(chart("Ngày", false)), movies(chart("Phim", true)),
      timer(new QTimer(this))
{
    setObjectName("reportView");
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);
    auto* content = new QWidget;
    scroll->setWidget(content);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* filters = HistoryUi::panel(layout, "Kỳ báo cáo");
    auto* row = new QHBoxLayout;
    row->addWidget(dates);
    auto* quick = new QComboBox;
    quick->setAccessibleName("Kỳ báo cáo nhanh");
    quick->addItems({"Chọn nhanh…", "Hôm nay", "7 ngày qua", "Tháng này"});
    row->addWidget(quick);
    row->addStretch();
    filters->addLayout(row);
    auto* controls = new QHBoxLayout;
    controls->addWidget(preview);
    controls->addStretch();
    auto* apply = new QPushButton("Áp dụng");
    auto* clear = new QPushButton("Xóa bộ lọc");
    controls->addWidget(apply);
    controls->addWidget(clear);
    filters->addLayout(controls);
    layout->addWidget(status);
    cards = new QGridLayout;
    layout->addLayout(cards);
    const QStringList titles{"Tổng tiền bán", "Tổng tiền hoàn", "Doanh thu thuần", "Vé bán trong kỳ", "Vé hủy trong kỳ"};
    for (int i = 0; i < int(metrics.size()); ++i) {
        auto* frame = new QFrame;
        frame->setObjectName("panel");
        auto* card = new QVBoxLayout(frame);
        card->addWidget(HistoryUi::label(titles[i], "description"));
        metrics[i] = HistoryUi::label("—", "priceValue");
        metrics[i]->setTextInteractionFlags(Qt::TextSelectableByMouse);
        metrics[i]->setAccessibleName(titles[i]);
        metrics[i]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        card->addWidget(metrics[i]);
        cards->addWidget(frame, i / 3, i % 3);
        cards->setColumnStretch(i % 3, 1);
    }
    charts = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(charts, 1);
    for (auto* table : {daily, movies}) {
        auto* column = new QVBoxLayout;
        charts->addLayout(column, 1);
        HistoryUi::panel(column, table == daily ? "Doanh thu thuần theo ngày"
                                                : "Top phim theo doanh thu vé còn SOLD")->addWidget(table);
    }
    layout->addWidget(HistoryUi::label("Tiền bán theo ngày lập hóa đơn, tiền hoàn theo ngày hủy. Xếp hạng chỉ tính vé còn SOLD của hóa đơn lập trong kỳ.", "description"));
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &ReportView::display);
    connect(apply, &QPushButton::clicked, this, &ReportView::load);
    connect(clear, &QPushButton::clicked, this, [this] { dates->setRange(); load(); });
    connect(preview, &QCheckBox::toggled, this, &ReportView::load);
    connect(quick, &QComboBox::activated, this, [this, quick](int choice) {
        const QDate today = QDate::currentDate();
        if (!choice) { return; }
        const QDate first = choice == 3 ? QDate(today.year(), today.month(), 1)
                                        : today.addDays(choice == 2 ? -6 : 0);
        dates->setRange(first, today);
        quick->setCurrentIndex(0);
        load();
    });
    load();
}

void ReportView::load()
{
    daily->setRowCount(0);
    movies->setRowCount(0);
    for (auto* metric : metrics) {
        metric->setText("—");
        metric->setToolTip({});
    }
    status->show();
    status->setText("Đang tải báo cáo…");
    timer->start(0);
}

void ReportView::display()
{
    const auto data = SalesHistory::report(auth, dates->period(), preview->isChecked());
    HistoryUi::showStatus(status, data.status, data.daily.isEmpty() && data.movies.isEmpty());
    if (data.status.state != SalesHistory::State::Available && data.status.state != SalesHistory::State::Preview) {
        return;
    }
    const QStringList values{formatVnd(data.sold), formatVnd(data.refunded), formatVnd(data.net),
                             QString::number(data.issued), QString::number(data.cancelled)};
    for (int i = 0; i < values.size(); ++i) {
        metrics[i]->setText(values[i]);
        metrics[i]->setToolTip(values[i]);
    }
    plot(daily, data.daily, false);
    plot(movies, data.movies, true);
}

void ReportView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const bool wide = event->size().width() >= 880;
    const int columns = wide ? 5 : 3;
    for (int i = 0; i < int(metrics.size()); ++i) {
        cards->addWidget(metrics[i]->parentWidget(), i / columns, i % columns);
        cards->setColumnStretch(i, i < columns ? 1 : 0);
    }
    charts->setDirection(wide ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
}
