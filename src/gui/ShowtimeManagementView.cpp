#include "ShowtimeManagementView.h"
#include "../business/ShowtimeService.h"
#include "../business/MovieService.h"
#include "../business/RoomService.h"
#include "../utils/DateTime.h"
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
const QString isoFormat = "yyyy-MM-dd'T'HH:mm:ss";

void addOption(QComboBox* box, const QString& caption, const QString& key)
{
    if (box->findData(key) < 0) {
        box->addItem(caption, key);
    }
}

void fillMovies(QComboBox* box, MovieService& movies, bool activeOnly)
{
    const int count = movies.getMovieCount();
    for (int i = 0; i < count; ++i) {
        const auto* m = movies.getMovieAt(i);
        if (m && (!activeOnly || m->isActive())) {
            const auto id = QString::fromStdString(m->getMovieId());
            addOption(box, QString::fromStdString(m->getTitle()) + " · " + id, id);
        }
    }
}

void fillRooms(QComboBox* box, RoomService& rooms, bool activeOnly)
{
    const int count = rooms.getRoomCount();
    for (int i = 0; i < count; ++i) {
        const auto* r = rooms.getRoomAt(i);
        if (r && (!activeOnly || r->isActive())) {
            const auto id = QString::fromStdString(r->getRoomId());
            addOption(box, QString::fromStdString(r->getName()) + " · " + id, id);
        }
    }
}

QString caption(QComboBox* box, const QString& id)
{
    const int index = box->findData(id);
    return index >= 0 ? box->itemText(index) : id;
}
}

ShowtimeManagementView::ShowtimeManagementView(ShowtimeService& service, MovieService& movies, RoomService& rooms, AuthService& auth)
    : CatalogView({"Mã suất", "Phim", "Phòng", "Bắt đầu", "Kết thúc", "Trạng thái"}),
      service(service), movies(movies), rooms(rooms), auth(auth),
      byMovie(new QComboBox), byRoom(new QComboBox), byDate(new QComboBox), byStatus(new QComboBox)
{
    action("Thêm suất chiếu", auth.hasPermission("SHOWTIME_ADD"), [this] { form(true); });
    action("Sửa", auth.hasPermission("SHOWTIME_UPDATE"), [this] { form(false); });
    pending("Hủy suất", auth.hasPermission("SHOWTIME_CANCEL"), "Chờ kiểm tra vé đã bán.");
    pending("Xóa suất", auth.hasPermission("SHOWTIME_DELETE"), "Chờ kiểm tra lịch sử vé.");
    auto* filters = new QHBoxLayout;
    auto addFilter = [&](const QString& title, QComboBox* box) {
        auto* stack = new QVBoxLayout;
        stack->addWidget(new QLabel(title));
        box->setAccessibleName(title);
        box->setMinimumContentsLength(8);
        box->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        stack->addWidget(box);
        filters->addLayout(stack, 1);
        connect(box, &QComboBox::currentIndexChanged, this, [this] { filter(); });
    };
    addFilter("Phim", byMovie);
    addFilter("Phòng", byRoom);
    addFilter("Ngày", byDate);
    addFilter("Trạng thái", byStatus);
    byStatus->addItem("Tất cả");
    byStatus->addItem("SCHEDULED", "SCHEDULED");
    byStatus->addItem("CANCELLED", "CANCELLED");
    byMovie->setObjectName("movieFilter");
    auto* reset = new QPushButton("Xóa bộ lọc");
    filters->addWidget(reset, 0, Qt::AlignBottom);
    connect(reset, &QPushButton::clicked, this, [this] {
        search->clear();
        for (auto* box : {byMovie, byRoom, byDate, byStatus}) {
            box->setCurrentIndex(0);
        }
    });
    static_cast<QVBoxLayout*>(layout())->insertLayout(1, filters);
}

void ShowtimeManagementView::refresh(const QString& movieId)
{
    CatalogUi::run(status, [&] {
        QSignalBlocker filmSignals(byMovie), roomSignals(byRoom), dateSignals(byDate);
        table->setRowCount(0);
        byMovie->clear();
        byRoom->clear();
        byDate->clear();
        byMovie->addItem("Tất cả phim");
        byRoom->addItem("Tất cả phòng");
        byDate->addItem("Tất cả ngày");
        if (auth.hasPermission("MOVIE_VIEW")) {
            fillMovies(byMovie, movies, false);
        }
        if (auth.hasPermission("ROOM_VIEW")) {
            fillRooms(byRoom, rooms, false);
        }
        const int count = service.getShowtimeCount();
        for (int i = 0; i < count; ++i) {
            const auto* show = service.getShowtimeAt(i);
            if (!show) {
                continue;
            }
            const auto movie = QString::fromStdString(show->getMovieId());
            const auto room = QString::fromStdString(show->getRoomId());
            const auto start = QString::fromStdString(show->getStartAt());
            const auto end = QString::fromStdString(show->getEndAt());
            const auto date = start.left(10);
            addOption(byMovie, movie, movie);
            addOption(byRoom, room, room);
            addOption(byDate, QDate::fromString(date, Qt::ISODate).toString("dd/MM/yyyy"), date);
            append({QString::fromStdString(show->getShowtimeId()), caption(byMovie, movie), caption(byRoom, room),
                    QString(start).replace('T', ' '), QString(end).replace('T', ' '),
                    show->getStatus() == ShowtimeStatus::SCHEDULED ? "SCHEDULED" : "CANCELLED"}, {}, {});
            const int row = table->rowCount() - 1;
            table->item(row, 1)->setData(Qt::UserRole, movie);
            table->item(row, 2)->setData(Qt::UserRole, room);
            table->item(row, 3)->setData(Qt::UserRole, date);
        }
        search->clear();
        byStatus->setCurrentIndex(0);
        byMovie->setCurrentIndex(qMax(0, byMovie->findData(movieId)));
        filter();
    });
}

void ShowtimeManagementView::filter()
{
    const auto query = search->text().trimmed();
    for (int i = 0; i < table->rowCount(); ++i) {
        bool textMatch = false;
        for (int col = 0; col < table->columnCount(); ++col) {
            textMatch |= table->item(i, col)->text().contains(query, Qt::CaseInsensitive);
        }
        const bool match = (byMovie->currentIndex() <= 0 || table->item(i, 1)->data(Qt::UserRole) == byMovie->currentData())
                        && (byRoom->currentIndex() <= 0 || table->item(i, 2)->data(Qt::UserRole) == byRoom->currentData())
                        && (byDate->currentIndex() <= 0 || table->item(i, 3)->data(Qt::UserRole) == byDate->currentData())
                        && (byStatus->currentIndex() <= 0 || table->item(i, 5)->text() == byStatus->currentData().toString());
        table->setRowHidden(i, !textMatch || !match);
    }
}

void ShowtimeManagementView::form(bool creating)
{
    const auto id = selectedId().toStdString();
    const auto* show = creating ? nullptr : service.findShowtimeById(id);
    if (!creating && (!show || show->getStatus() != ShowtimeStatus::SCHEDULED ||
        !DateTime::isBefore(DateTime::getCurrentDateTime(), show->getStartAt()))) {
        status->setText("Chọn một suất SCHEDULED chưa bắt đầu để chỉnh sửa.");
        return;
    }
    if (creating && (!auth.hasPermission("MOVIE_VIEW") || !auth.hasPermission("ROOM_VIEW"))) {
        status->setText("Cần quyền xem danh mục phim và phòng để chọn suất mới.");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(creating ? "Thêm suất chiếu" : "Sửa suất chiếu");
    QFormLayout fields(&dialog);
    fields.addRow("Mã suất", new QLabel(creating ? "Hệ thống cấp" : QString::fromStdString(id)));
    QComboBox movie, room;
    if (creating) {
        fillMovies(&movie, movies, true);
        if (auth.hasPermission("ROOM_VIEW")) {
            fillRooms(&room, rooms, true);
        }
    } else {
        const auto filmId = QString::fromStdString(show->getMovieId());
        const auto roomId = QString::fromStdString(show->getRoomId());
        movie.addItem(caption(byMovie, filmId), filmId);
        room.addItem(caption(byRoom, roomId), roomId);
        movie.setEnabled(false);
        room.setEnabled(false);
    }
    fields.addRow("Phim", &movie);
    fields.addRow("Phòng", &room);
    QDateTimeEdit start(creating ? QDateTime::currentDateTime().addDays(1)
                                 : QDateTime::fromString(QString::fromStdString(show->getStartAt()), isoFormat));
    start.setDisplayFormat("dd/MM/yyyy HH:mm:ss");
    start.setCalendarPopup(true);
    fields.addRow("Ngày và giờ bắt đầu", &start);
    QLabel end;
    std::string endAt;
    // With SHOWTIME_UPDATE alone the existing slot length is visible via ShowtimeService.
    // The service still validates the submitted end against the current movie duration.
    const auto slotSeconds = creating ? 0 : start.dateTime().secsTo(
        QDateTime::fromString(QString::fromStdString(show->getEndAt()), isoFormat));
    auto preview = [&] {
        endAt.clear();
        end.clear();
        CatalogUi::run(&end, [&] {
            if (auth.hasPermission("MOVIE_VIEW")) {
                const auto* film = movies.findMovieById(movie.currentData().toString().toStdString());
                if (!film) {
                    end.setText("Không tìm thấy phim.");
                    return;
                }
                endAt = DateTime::addMinutes(start.dateTime().toString(isoFormat).toStdString(), film->getDurationMinutes());
            } else {
                endAt = start.dateTime().addSecs(slotSeconds).toString(isoFormat).toStdString();
            }
            end.setText(QString::fromStdString(endAt).replace('T', ' '));
        });
    };
    connect(&start, &QDateTimeEdit::dateTimeChanged, &dialog, preview);
    connect(&movie, &QComboBox::currentIndexChanged, &dialog, preview);
    preview();
    fields.addRow("Kết thúc", &end);
    fields.addRow("Trạng thái", new QLabel("SCHEDULED"));
    if (!CatalogUi::confirmForm(dialog, fields, creating)) {
        return;
    }
    const Showtime values(id, movie.currentData().toString().toStdString(), room.currentData().toString().toStdString(),
                          start.dateTime().toString(isoFormat).toStdString(), endAt, ShowtimeStatus::SCHEDULED);
    const bool changed = service.updateShowtime(values);
    CatalogUi::result(status, changed);
    if (changed) {
        refresh(byMovie->currentData().toString());
    } else {
        status->setText("Không thể đổi lịch. Kiểm tra thời điểm bắt đầu, khoảng phát hành và khoảng nghỉ 15 phút trong phòng.");
    }
}
