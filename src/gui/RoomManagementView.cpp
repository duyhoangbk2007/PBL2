#include "RoomManagementView.h"
#include "../business/RoomService.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QTableWidget>

RoomManagementView::RoomManagementView(RoomService& service, AuthService& auth)
    : CatalogView({"Mã phòng", "Tên phòng", "Hàng", "Cột", "Trạng thái", "Sơ đồ"}),
      service(service), auth(auth)
{
    pending("Thêm phòng", auth.hasPermission("ROOM_ADD"), "Đang chờ Service cấp mã phòng.");
    action("Sửa", auth.hasPermission("ROOM_UPDATE"), [this] { edit(); });
    pending("Bật / tắt", auth.hasPermission("ROOM_UPDATE"), "Chưa có API thay đổi trạng thái phòng trên nhánh này.");
    action("Xóa", auth.hasPermission("ROOM_DELETE"), [this] { remove(); });
    refresh();
}

void RoomManagementView::refresh()
{
    CatalogUi::run(status, [this] {
        table->setRowCount(0);
        const int count = service.getRoomCount();
        for (int i = 0; i < count; ++i) {
            const auto* item = service.getRoomAt(i);
            if (!item) {
                continue;
            }
            const QString id = QString::fromStdString(item->getRoomId());
            append({id, QString::fromStdString(item->getName()), QString::number(item->getRowCount()),
                    QString::number(item->getColumnCount()),
                    item->isActive() ? "Hoạt động" : "Ngừng hoạt động"},
                   auth.hasPermission("SEAT_VIEW") ? "Xem ghế →" : "", id);
        }
        filter();
    });
}

void RoomManagementView::edit()
{
    const auto key = selectedId().toStdString();
    const auto* item = service.findRoomById(key);
    if (!item) {
        status->setText("Hãy chọn một phòng trong danh sách.");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("Sửa phòng");
    QFormLayout form(&dialog);
    form.addRow("Mã phòng", new QLabel(QString::fromStdString(key)));
    QLineEdit title(QString::fromStdString(item->getName()));
    form.addRow("Tên phòng", &title);
    auto* rows = CatalogUi::number(form, "Số hàng", item->getRowCount(), 1);
    auto* columns = CatalogUi::number(form, "Số cột", item->getColumnCount(), 1);
    rows->setReadOnly(item->getShowtimeCount() > 0);
    columns->setReadOnly(item->getShowtimeCount() > 0);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    const bool active = item->isActive();
    if (dialog.exec() == QDialog::Accepted) {
        Room values(key, title.text().toStdString(), rows->value(), columns->value(), active);
        CatalogUi::result(status, service.updateRoom(values));
        refresh();
    }
}

void RoomManagementView::remove()
{
    const QString id = selectedId();
    if (id.isEmpty()) {
        status->setText("Hãy chọn một phòng trong danh sách.");
        return;
    }
    if (QMessageBox::question(this, "Xóa phòng", "Xóa phòng " + id + "?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        const bool removed = service.removeRoom(id.toStdString());
        CatalogUi::result(status, removed);
        if (!removed) {
            status->setText("Không thể xóa phòng. Kiểm tra ghế hoặc suất chiếu liên kết.");
        }
        refresh();
    }
}
