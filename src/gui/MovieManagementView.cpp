#include "MovieManagementView.h"
#include "../business/MovieService.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QTableWidget>

MovieManagementView::MovieManagementView(MovieService& service, AuthService& auth)
    : CatalogView({"Mã phim", "Tên phim", "Thể loại", "Phút", "Tuổi", "Khởi chiếu", "Kết thúc", "Trạng thái", "Suất chiếu"}),
      service(service), auth(auth)
{
    action("Thêm phim", auth.hasPermission("MOVIE_ADD"), [this] { add(); });
    action("Chi tiết", auth.hasPermission("MOVIE_VIEW"), [this] { detail(); });
    action("Sửa", auth.hasPermission("MOVIE_UPDATE"), [this] { edit(); });
    action("Bật / tắt", auth.hasPermission("MOVIE_UPDATE"), [this] {
        const auto id = selectedId().toStdString();
        const auto* film = this->service.findMovieById(id);
        if (!film) {
            status->setText("Hãy chọn một phim trong danh sách.");
            return;
        }
        const bool activate = !film->isActive();
        const QString caption = activate ? "Bật hoạt động" : "Ngừng hoạt động";
        if (QMessageBox::question(this, caption, caption + " phim " + QString::fromStdString(id) + "?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
            CatalogUi::result(status, this->service.setMovieActive(id, activate));
            refresh();
        }
    });
    action("Xóa", auth.hasPermission("MOVIE_DELETE"), [this] { remove(); });
    refresh();
}

void MovieManagementView::add()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Thêm phim");
    QFormLayout form(&dialog);
    QLineEdit title, genre, release, end;
    form.addRow("Tên phim", &title);
    form.addRow("Thể loại", &genre);
    auto* duration = CatalogUi::number(form, "Thời lượng (phút)", 90, 1);
    auto* age = CatalogUi::number(form, "Tuổi tối thiểu", 0, 0);
    form.addRow("Khởi chiếu (YYYY-MM-DD)", &release);
    form.addRow("Kết thúc (YYYY-MM-DD)", &end);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) { return; }
    Movie values("", title.text().toStdString(), genre.text().toStdString(), duration->value(),
                 age->value(), release.text().toStdString(), end.text().toStdString(), true);
    const bool added = service.addMovie(&values);
    CatalogUi::result(status, added);
    if (added) { refresh(); }
}

void MovieManagementView::detail()
{
    const auto* item = service.findMovieById(selectedId().toStdString());
    if (!item) {
        status->setText("Hãy chọn một phim còn tồn tại trong danh sách.");
        refresh();
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("Chi tiết phim");
    QFormLayout form(&dialog);
    form.addRow("Mã phim", new QLabel(QString::fromStdString(item->getMovieId())));
    form.addRow("Tên phim", new QLabel(QString::fromStdString(item->getTitle())));
    form.addRow("Thể loại", new QLabel(QString::fromStdString(item->getGenre())));
    form.addRow("Thời lượng", new QLabel(QString::number(item->getDurationMinutes()) + " phút"));
    form.addRow("Tuổi tối thiểu", new QLabel(QString::number(item->getMinAge())));
    form.addRow("Khởi chiếu", new QLabel(QString::fromStdString(item->getReleaseDate())));
    form.addRow("Kết thúc", new QLabel(QString::fromStdString(item->getEndDate())));
    form.addRow("Trạng thái", new QLabel(item->isActive() ? "Hoạt động" : "Ngừng hoạt động"));
    QDialogButtonBox buttons(QDialogButtonBox::Close);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

void MovieManagementView::refresh()
{
    CatalogUi::run(status, [this] {
        table->setRowCount(0);
        const int count = service.getMovieCount();
        for (int i = 0; i < count; ++i) {
            const auto* item = service.getMovieAt(i);
            if (!item) {
                continue;
            }
            const QString id = QString::fromStdString(item->getMovieId());
            append({id, QString::fromStdString(item->getTitle()), QString::fromStdString(item->getGenre()),
                    QString::number(item->getDurationMinutes()), QString::number(item->getMinAge()),
                    QString::fromStdString(item->getReleaseDate()), QString::fromStdString(item->getEndDate()),
                    item->isActive() ? "Hoạt động" : "Ngừng hoạt động"},
                   auth.hasPermission("SHOWTIME_VIEW") ? "Xem suất chiếu →" : "", id);
        }
        filter();
    });
}

void MovieManagementView::edit()
{
    const auto key = selectedId().toStdString();
    const auto* item = service.findMovieById(key);
    if (!item) {
        status->setText("Hãy chọn một phim trong danh sách.");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("Sửa phim");
    QFormLayout form(&dialog);
    form.addRow("Mã phim", new QLabel(QString::fromStdString(key)));
    QLineEdit title(QString::fromStdString(item->getTitle()));
    form.addRow("Tên phim", &title);
    QLineEdit genre(QString::fromStdString(item->getGenre()));
    form.addRow("Thể loại", &genre);
    auto* duration = CatalogUi::number(form, "Thời lượng (phút)", item->getDurationMinutes(), 1);
    auto* age = CatalogUi::number(form, "Tuổi tối thiểu", item->getMinAge(), 0);
    QLineEdit release(QString::fromStdString(item->getReleaseDate()));
    QLineEdit end(QString::fromStdString(item->getEndDate()));
    form.addRow("Khởi chiếu (YYYY-MM-DD)", &release);
    form.addRow("Kết thúc (YYYY-MM-DD)", &end);
    duration->setReadOnly(item->getShowtimeCount() > 0);
    release.setReadOnly(item->getShowtimeCount() > 0);
    end.setReadOnly(item->getShowtimeCount() > 0);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    const bool active = item->isActive();
    if (dialog.exec() == QDialog::Accepted) {
        Movie values(key, title.text().toStdString(), genre.text().toStdString(), duration->value(), age->value(),
                     release.text().toStdString(), end.text().toStdString(), active);
        CatalogUi::result(status, service.updateMovie(values));
        refresh();
    }
}

void MovieManagementView::remove()
{
    const QString id = selectedId();
    if (id.isEmpty()) {
        status->setText("Hãy chọn một phim trong danh sách.");
        return;
    }
    if (QMessageBox::question(this, "Xóa phim", "Xóa phim " + id + "?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        const bool removed = service.removeMovie(id.toStdString());
        CatalogUi::result(status, removed);
        if (!removed) {
            status->setText("Không thể xóa phim. Kiểm tra suất chiếu liên kết.");
        }
        refresh();
    }
}
