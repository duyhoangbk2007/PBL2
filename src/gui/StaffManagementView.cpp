#include "StaffManagementView.h"
#include "../business/StaffService.h"
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QMessageBox>

namespace {
// StaffService expects a complete User. Only the adapter handles the stored credential.
bool updateProfile(StaffService& service, const std::string& id,
                   const std::string& username, const std::string& name)
{
    const auto* current = service.findStaffById(id);
    if (!current) {
        return false;
    }
    return service.updateStaff(User(id, username, current->getPassword(), name,
                                    current->getRole(), current->isActive()));
}
}

StaffManagementView::StaffManagementView(StaffService& service, AuthService& auth)
    : CatalogView({"Mã nhân viên", "Họ tên", "Username", "Vai trò", "Trạng thái"}), service(service)
{
    search->setPlaceholderText("Tìm theo mã, họ tên hoặc username…");
    action("Thêm nhân viên", auth.hasPermission("STAFF_ADD"), [this] { form(true); });
    action("Sửa", auth.hasPermission("STAFF_UPDATE"), [this] { form(false); });
    action("Vô hiệu hóa", auth.hasPermission("STAFF_DEACTIVATE"), [this] { deactivate(); });
}

void StaffManagementView::refresh()
{
    CatalogUi::run(status, [this] {
        table->setRowCount(0);
        const int count = service.getStaffCount();
        for (int i = 0; i < count; ++i) {
            const auto* item = service.getStaffAt(i);
            if (item) {
                append({QString::fromStdString(item->getUserId()), QString::fromStdString(item->getFullName()),
                        QString::fromStdString(item->getUsername()), "STAFF",
                        item->isActive() ? "Hoạt động" : "Đã vô hiệu hóa"}, {}, {});
            }
        }
        filter();
    });
}

void StaffManagementView::filter()
{
    CatalogUi::run(status, [this] {
        const QString query = search->text().trimmed();
        for (int row = 0; row < table->rowCount(); ++row) {
            const QString id = table->item(row, 0)->text();
            bool visible = id.contains(query, Qt::CaseInsensitive)
                        || table->item(row, 1)->text().contains(query, Qt::CaseInsensitive)
                        || table->item(row, 2)->text().contains(query, Qt::CaseInsensitive);
            table->setRowHidden(row, !visible);
        }
    });
}

void StaffManagementView::form(bool creating)
{
    const auto id = selectedId().toStdString();
    const auto* current = creating ? nullptr : service.findStaffById(id);
    if (!creating && !current) {
        status->setText("Hãy chọn một nhân viên.");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(creating ? "Thêm nhân viên" : "Sửa nhân viên");
    QFormLayout fields(&dialog);
    fields.addRow("Mã nhân viên", new QLabel(creating ? "Hệ thống cấp" : QString::fromStdString(id)));
    QLineEdit name(current ? QString::fromStdString(current->getFullName()) : QString());
    fields.addRow("Họ tên", &name);
    QLineEdit username(current ? QString::fromStdString(current->getUsername()) : QString());
    fields.addRow("Username", &username);
    QLineEdit password;
    if (creating) {
        password.setEchoMode(QLineEdit::Password);
        fields.addRow("Mật khẩu khởi tạo", &password);
    }
    fields.addRow("Vai trò", new QLabel("STAFF"));
    fields.addRow("Trạng thái", new QLabel(!current || current->isActive() ? "Hoạt động" : "Đã vô hiệu hóa"));
    if (!CatalogUi::confirmForm(dialog, fields, creating)) {
        return;
    }
    const auto login = username.text().toStdString();
    const auto* duplicate = service.findStaffByUsername(login);
    if (duplicate && duplicate->getUserId() != id) {
        status->setText("Username đã được sử dụng.");
        return;
    }
    CatalogUi::result(status, updateProfile(service, id, login, name.text().toStdString()));
    refresh();
}

void StaffManagementView::deactivate()
{
    const auto id = selectedId();
    const auto* current = service.findStaffById(id.toStdString());
    if (!current || !current->isActive()) {
        status->setText("Hãy chọn một tài khoản nhân viên đang hoạt động.");
        return;
    }
    if (QMessageBox::question(this, "Vô hiệu hóa nhân viên", "Vô hiệu hóa tài khoản " + id + "?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        CatalogUi::result(status, service.deactivateStaff(id.toStdString()));
        refresh();
    }
}
