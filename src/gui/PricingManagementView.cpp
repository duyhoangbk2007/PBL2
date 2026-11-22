#include "PricingManagementView.h"
#include "../business/PricingService.h"
#include "CatalogView.h"
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>
#include <limits>

namespace {
const QStringList captions = {"Ghế thường", "Ghế VIP", "Ghế đôi", "Học sinh", "Thành viên"};
const SeatType seatTypes[] = {SeatType::STANDARD, SeatType::VIP, SeatType::COUPLE};
}

QString formatVnd(double value)
{
    const QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
    QString result = locale.toString(value, 'f', 6);
    while (result.endsWith('0')) {
        result.chop(1);
    }
    if (result.endsWith(locale.decimalPoint())) {
        result.chop(1);
    }
    return result + " ₫";
}

PricingManagementView::PricingManagementView(PricingService& pricing, AuthService& auth)
    : pricing(pricing), auth(auth), apply(new QPushButton("Lưu thay đổi")), message(new QLabel)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* row = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        auto* card = new QFrame;
        card->setObjectName("panel");
        auto* body = new QVBoxLayout(card);
        body->setContentsMargins(18, 15, 18, 15);
        body->addWidget(new QLabel(captions[i]));
        cards[i] = new QLabel;
        cards[i]->setObjectName("priceValue");
        cards[i]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        cards[i]->setWordWrap(true);
        body->addWidget(cards[i]);
        row->addWidget(card);
    }
    layout->addLayout(row);
    auto* panels = new QHBoxLayout;
    auto* editor = new QFrame;
    editor->setObjectName("panel");
    auto* form = new QFormLayout(editor);
    form->setContentsMargins(18, 18, 18, 18);
    form->addRow(new QLabel("Giá vé theo loại ghế"));
    for (int i = 0; i < 5; ++i) {
        if (i == 3) {
            form->addRow(new QLabel("Tỷ lệ giảm giá"));
        }
        fields[i] = new QDoubleSpinBox;
        fields[i]->setDecimals(6);
        fields[i]->setRange(i < 3 ? 0.000001 : 0, i < 3 ? std::numeric_limits<double>::max() : 100);
        fields[i]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        fields[i]->setLocale(QLocale(QLocale::Vietnamese));
        fields[i]->setSuffix(i < 3 ? " VND" : " %");
        form->addRow(captions[i], fields[i]);
    }
    auto* reset = new QPushButton("Khôi phục biểu mẫu");
    form->addRow(reset, apply);
    reset->setVisible(auth.hasPermission("PRICING_UPDATE"));
    apply->setVisible(auth.hasPermission("PRICING_UPDATE"));
    panels->addWidget(editor, 1);
    auto* rules = new QLabel("Quy tắc tính giá\n\nƯu đãi học sinh và thành viên không cộng dồn. Áp dụng mức cao nhất.\n\nMỗi 10.000 VND thực trả trên từng vé được 1 điểm cho thành viên.\n\nMột vé COUPLE dành cho hai người.");
    rules->setObjectName("priceRules");
    rules->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    rules->setWordWrap(true);
    panels->addWidget(rules, 1);
    layout->addLayout(panels);
    message->setWordWrap(true);
    layout->addWidget(message);
    layout->addStretch();
    connect(reset, &QPushButton::clicked, this, &PricingManagementView::refresh);
    connect(apply, &QPushButton::clicked, this, &PricingManagementView::save);
}

void PricingManagementView::refresh()
{
    message->clear();
    apply->setEnabled(false);
    for (auto* field : fields) {
        field->setEnabled(false);
    }
    CatalogUi::run(message, [this] {
        const auto* p = pricing.getPricing();
        if (!p) {
            message->setText("Bảng giá chưa được nạp.");
            return;
        }
        const double values[] = {p->getStandardPrice(), p->getVipPrice(), p->getCouplePrice(),
                                 p->getStudentDiscount(), p->getMemberDiscount()};
        const bool permitted = auth.hasPermission("PRICING_UPDATE");
        bool representable = true;
        for (int i = 0; i < 5; ++i) {
            fields[i]->setValue(values[i]);
            displayed[i] = fields[i]->value();
            const bool exact = displayed[i] == values[i];
            representable &= exact;
            const QString source = QLocale(QLocale::Vietnamese).toString(values[i], 'g', 17);
            fields[i]->setToolTip(exact ? QString() : "Giá trị trong dữ liệu: " + source);
            if (i < 3) {
                cards[i]->setText(exact ? formatVnd(values[i]) : source + " ₫");
                cards[i]->setToolTip(cards[i]->text());
            }
        }
        for (auto* field : fields) { field->setEnabled(permitted && representable); }
        apply->setEnabled(permitted && representable);
        if (!representable) {
            message->setText("Bảng giá có giá trị vượt độ chính xác lưu 6 chữ số thập phân. "
                             "Đã khóa chỉnh sửa để giữ nguyên dữ liệu.");
        }
    });
}

void PricingManagementView::save()
{
    if (!apply->isEnabled() || !auth.hasPermission("PRICING_UPDATE")) { return; }
    QStringList saved;
    QString failed;
    for (int i = 0; i < 5; ++i) {
        const double value = fields[i]->value();
        if (displayed[i] == value) {
            continue;
        }
        try {
            const bool ok = i < 3 ? pricing.updateSeatPrice(seatTypes[i], value)
                : pricing.updateDiscountRate(i == 3 ? DiscountType::STUDENT : DiscountType::MEMBER, value);
            if (ok) {
                saved << captions[i];
                continue;
            }
        } catch (const std::exception&) {
            // Earlier calls may already have persisted their own field.
        }
        failed = captions[i];
        break;
    }
    refresh();
    QString result = saved.isEmpty() ? "Chưa ghi thay đổi." : "Đã ghi: " + saved.join(", ") + ".";
    if (!failed.isEmpty()) {
        result += " Dừng tại " + failed + ". Các thay đổi còn lại chưa được ghi.";
    }
    message->setText(result);
}
