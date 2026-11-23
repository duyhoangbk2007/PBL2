#include "MemberManagementView.h"
#include "../business/MemberService.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QRegularExpressionValidator>

MemberManagementView::MemberManagementView(MemberService& service, AuthService& auth)
    : CatalogView({"Mã thành viên", "Họ tên", "Số điện thoại", "Điểm tích lũy"}), service(service)
{
    search->setPlaceholderText("Tìm theo mã, họ tên hoặc số điện thoại…");
    action("Thêm thành viên", auth.hasPermission("MEMBER_ADD"), [this] { form(true); });
    action("Sửa", auth.hasPermission("MEMBER_UPDATE"), [this] { form(false); });
}

void MemberManagementView::refresh()
{
    CatalogUi::run(status, [this] {
        table->setRowCount(0);
        const int count = service.getMemberCount();
        for (int i = 0; i < count; ++i) {
            const auto* item = service.getMemberAt(i);
            if (item) {
                append({QString::fromStdString(item->getMemberId()), QString::fromStdString(item->getFullName()),
                        QString::fromStdString(item->getPhone()), QString::number(item->getPoints())}, {}, {});
            }
        }
        filter();
    });
}

void MemberManagementView::filter()
{
    CatalogUi::run(status, [this] {
        const QString query = search->text().trimmed();
        const Member* exact = nullptr;
        if (!query.isEmpty()) {
            exact = service.findMemberById(query.toStdString());
            if (!exact) {
                exact = service.findMemberByPhone(query.toStdString());
            }
        }
        for (int row = 0; row < table->rowCount(); ++row) {
            const QString id = table->item(row, 0)->text();
            bool visible = id.contains(query, Qt::CaseInsensitive)
                        || table->item(row, 1)->text().contains(query, Qt::CaseInsensitive)
                        || table->item(row, 2)->text().contains(query, Qt::CaseInsensitive);
            if (exact) {
                visible = id.toStdString() == exact->getMemberId();
            }
            table->setRowHidden(row, !visible);
        }
    });
}

void MemberManagementView::form(bool creating)
{
    const auto id = selectedId().toStdString();
    const auto* current = creating ? nullptr : service.findMemberById(id);
    if (!creating && !current) {
        status->setText("Hãy chọn một thành viên.");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(creating ? "Thêm thành viên" : "Sửa thành viên");
    QFormLayout fields(&dialog);
    fields.addRow("Mã thành viên", new QLabel(creating ? "Hệ thống cấp" : QString::fromStdString(id)));
    QLineEdit name(current ? QString::fromStdString(current->getFullName()) : QString());
    fields.addRow("Họ tên", &name);
    QLineEdit phone(current ? QString::fromStdString(current->getPhone()) : QString());
    phone.setValidator(new QRegularExpressionValidator(QRegularExpression("0[0-9]{9}"), &phone));
    fields.addRow("Số điện thoại", &phone);
    const int points = current ? current->getPoints() : 0;
    fields.addRow("Điểm tích lũy", new QLabel(QString::number(points)));
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    fields.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    if (!phone.hasAcceptableInput()) {
        status->setText("Số điện thoại cần đúng 10 chữ số và bắt đầu bằng 0.");
        return;
    }
    bool changed = false;
    if (creating) {
        Member candidate("", name.text().toStdString(), phone.text().toStdString(), 0);
        changed = service.addMember(&candidate);
    } else {
        const auto* latest = service.findMemberById(id);
        if (!latest) {
            status->setText("Thành viên không còn tồn tại. Danh sách đã được cập nhật.");
            refresh();
            return;
        }
        changed = service.updateMember(Member(id, name.text().toStdString(),
                                              phone.text().toStdString(), latest->getPoints()));
    }
    CatalogUi::result(status, changed);
    refresh();
}
