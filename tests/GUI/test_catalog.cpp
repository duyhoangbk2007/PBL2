#include "test_catalog.h"
#include "MainWindow.h"
#include "MovieManagementView.h"
#include "ApplicationContext.h"

#include <QApplication>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDateTimeEdit>
#include <memory>
#include <QLineEdit>
#include <QMessageBox>
#include <QMap>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTest>
#include <QTimer>
#include <functional>

namespace {
QWidget* page(MainWindow& window, const QString& permission)
{
    for (auto* button : window.findChild<QButtonGroup*>()->buttons()) {
        if (button->property("permission") == permission) {
            button->click();
            QApplication::processEvents();
            return window.findChild<QStackedWidget*>("pages")->currentWidget();
        }
    }
    return nullptr;
}

QPushButton* button(QWidget* parent, const QString& text)
{
    for (auto* item : parent->findChildren<QPushButton*>()) {
        if (item->text() == text) {
            return item;
        }
    }
    return nullptr;
}

bool snapshot(MainWindow& window, const QString& name)
{
    QApplication::processEvents();
    const auto dir = qEnvironmentVariable("PBL2_CATALOG_SCREENSHOTS");
    return dir.isEmpty() || window.grab().save(QDir(dir).filePath(name + ".png"));
}

void editSeat(QTableView* grid, int row, int column, const std::function<void(QDialog*)>& interact)
{
    const auto index = grid->model()->index(row, column);
    grid->scrollTo(index);
    QTimer::singleShot(0, grid, [interact] {
        if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
            interact(dialog);
        }
    });
    QTest::mouseClick(grid->viewport(), Qt::LeftButton, Qt::NoModifier, grid->visualRect(index).center());
}

void confirmDeletion(QMessageBox::StandardButton answer)
{
    QTimer::singleShot(0, [answer] {
        if (auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            box->button(answer)->click();
        }
    });
}
}

namespace {
bool modalAction(QWidget* view, const QString& action, const std::function<bool(QDialog*)>& check)
{
    auto* trigger = button(view, action);
    if (!trigger) {
        return false;
    }
    bool passed = false;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog) {
            passed = check(dialog);
            if (dialog->isVisible()) {
                dialog->reject();
            }
        }
    });
    timer.start(0);
    trigger->click();
    return passed;
}

bool pendingForm(QWidget* view, const QString& name)
{
    return modalAction(view, name, [](QDialog* dialog) {
        auto* ok = dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok);
        return !ok->isEnabled();
    });
}

bool testPeople(MainWindow& window, ApplicationContext& context)
{
    auto* staff = page(window, "STAFF_VIEW");
    auto* table = staff->findChild<QTableWidget*>();
    if (table->rowCount() != 3 || !snapshot(window, "nhan-vien") || !pendingForm(staff, "Thêm nhân viên")) {
        return false;
    }
    auto* query = staff->findChild<QLineEdit*>();
    query->setText("cashier");
    if (table->isRowHidden(0) || !table->isRowHidden(1)) {
        return false;
    }
    query->clear();
    table->selectRow(0);
    if (!modalAction(staff, "Sửa", [](QDialog* dialog) {
        auto fields = dialog->findChildren<QLineEdit*>();
        if (fields.size() != 2) {
            return false;
        }
        fields[0]->setText("Nhan vien moi");
        fields[1]->setText("cashier2");
        dialog->accept();
        return true;
    }) || !context.staff.findStaffByUsername("cashier2")) {
        return false;
    }
    table->selectRow(2);
    confirmDeletion(QMessageBox::No);
    button(staff, "Vô hiệu hóa")->click();
    if (!context.staff.findStaffById("USR0004")->isActive()) {
        return false;
    }
    confirmDeletion(QMessageBox::Yes);
    button(staff, "Vô hiệu hóa")->click();
    if (context.staff.findStaffById("USR0004")->isActive()) {
        return false;
    }
    auto* members = page(window, "MEMBER_VIEW");
    table = members->findChild<QTableWidget*>();
    if (!table || table->rowCount() != 2 || !snapshot(window, "thanh-vien") || !pendingForm(members, "Thêm thành viên")) {
        return false;
    }
    query = members->findChild<QLineEdit*>();
    for (const QString term : {"MEM0001", "0900000001", "Minh Anh"}) {
        query->setText(term);
        if (table->isRowHidden(0) || !table->isRowHidden(1)) {
            return false;
        }
    }
    query->clear();
    table->selectRow(0);
    if (!modalAction(members, "Sửa", [](QDialog* dialog) {
        auto fields = dialog->findChildren<QLineEdit*>();
        if (fields.size() != 2) {
            return false;
        }
        fields[0]->setText("Thanh vien moi");
        fields[1]->setText("0900000008");
        dialog->accept();
        return true;
    })) {
        return false;
    }
    table->selectRow(0);
    if (!modalAction(members, "Sửa", [](QDialog* dialog) {
        dialog->findChildren<QLineEdit*>().at(1)->setText("0900000002");
        dialog->accept();
        return true;
    })) {
        return false;
    }
    const auto* updated = context.members.findMemberByPhone("0900000008");
    return updated && updated->getFullName() == "Thanh vien moi" && updated->getPoints() == 18;
}

bool testSchedule(MainWindow& window, ApplicationContext& context)
{
    // Seed a future show in memory so the edit test remains valid beyond the fixture dates.
    auto movie = std::make_unique<Movie>("MOV0099", "Phim thu lich", "Action", 90, 13, "2020-01-01", "2099-12-31", true);
    if (!context.movies.addMovie(movie.get())) {
        return false;
    }
    const auto* storedMovie = context.movies.getMovieAt(context.movies.getMovieCount() - 1);
    if (!storedMovie) { return false; }
    const auto movieId = storedMovie->getMovieId();
    const auto start = QDateTime(QDate::currentDate().addDays(12), QTime(9, 0));
    const auto format = QString("yyyy-MM-dd'T'HH:mm:ss");
    auto show = std::make_unique<Showtime>("", movieId, "ROM0001", start.toString(format).toStdString(),
        start.addSecs(5400).toString(format).toStdString(), ShowtimeStatus::SCHEDULED);
    if (!context.showtimes.addShowtime(show.get())) {
        return false;
    }
    const auto* storedShow = context.showtimes.getShowtimeAt(context.showtimes.getShowtimeCount() - 1);
    if (!storedShow) { return false; }
    const auto showId = storedShow->getShowtimeId();
    auto* filmPage = page(window, "MOVIE_VIEW");
    // The new film and its future show are seeded through Services after the session opened.
    auto* toggleList = filmPage->findChild<QTableWidget*>();
    auto* movieView = dynamic_cast<MovieManagementView*>(filmPage->findChild<CatalogView*>());
    if (!movieView) { return false; }
    movieView->refresh();
    toggleList->selectRow(toggleList->rowCount() - 1);
    confirmDeletion(QMessageBox::Yes);
    button(filmPage, "Bật / tắt")->click();
    if (!context.movies.findMovieById(movieId)->isActive()) { return false; }
    auto* view = page(window, "SHOWTIME_VIEW");
    auto* table = view->findChild<QTableWidget*>();
    if (!table || table->rowCount() != 4 || !snapshot(window, "suat-chieu") || !pendingForm(view, "Thêm suất chiếu") ||
        button(view, "Hủy suất")->isEnabled() || button(view, "Xóa suất")->isEnabled()) {
        return false;
    }
    for (auto* box : view->findChildren<QComboBox*>()) {
        const QMap<QString, QString> criteria = {{"Phim", "MOV0002"}, {"Phòng", "ROM0002"},
                                                {"Ngày", "2026-11-26"}, {"Trạng thái", "CANCELLED"}};
        box->setCurrentIndex(box->findData(criteria.value(box->accessibleName())));
    }
    if (!table->isRowHidden(0) || table->isRowHidden(1) || !table->isRowHidden(2) || !table->isRowHidden(3)) {
        return false;
    }
    button(view, "Xóa bộ lọc")->click();
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->isRowHidden(row)) {
            return false;
        }
    }
    for (int row : {1, 2}) {
        table->selectRow(row);
        if (modalAction(view, "Sửa", [](QDialog*) { return true; })) {
            return false;
        }
    }
    table->selectRow(3);
    auto editTime = [&](const QDateTime& value) {
        return modalAction(view, "Sửa", [&](QDialog* dialog) {
            for (auto* box : dialog->findChildren<QComboBox*>()) {
                if (box->isEnabled()) {
                    return false;
                }
            }
            dialog->findChild<QDateTimeEdit*>()->setDateTime(value);
            dialog->accept();
            return true;
        });
    };
    if (!editTime(start.addYears(-25)) || context.showtimes.findShowtimeById(showId)->getStartAt() != start.toString(format).toStdString()) {
        return false;
    }
    // A second show starts two hours after the first. Moving by 20 minutes
    // leaves only 10 minutes between shows; moving by 15 leaves the allowed gap.
    const auto next = start.addSecs(7200);
    auto second = std::make_unique<Showtime>("", movieId, "ROM0001", next.toString(format).toStdString(),
        next.addSecs(5400).toString(format).toStdString(), ShowtimeStatus::SCHEDULED);
    if (!context.showtimes.addShowtime(second.get())) {
        return false;
    }
    if (!editTime(start.addSecs(1200)) || context.showtimes.findShowtimeById(showId)->getStartAt() != start.toString(format).toStdString()) {
        return false;
    }
    const auto later = start.addSecs(900);
    return editTime(later) && context.showtimes.findShowtimeById(showId)->getEndAt() == later.addSecs(5400).toString(format).toStdString();
}
}

bool testCatalog(MainWindow& window, ApplicationContext& context)
{
    window.resize(1280, 820);
    auto* movies = page(window, "MOVIE_VIEW");
    auto* table = movies->findChild<QTableWidget*>();
    auto* search = movies->findChild<QLineEdit*>();
    if (!table || !search || table->rowCount() != 3 || !snapshot(window, "phim")) {
        return false;
    }
    search->setText("phương Bắc");
    if (!table->isRowHidden(0) || table->isRowHidden(1)) {
        return false;
    }
    search->clear();
    for (const bool expected : {false, true}) {
        table->selectRow(0);
        confirmDeletion(QMessageBox::Yes);
        button(movies, "Bật / tắt")->click();
        if (context.movies.findMovieById("MOV0001")->isActive() != expected) { return false; }
    }
    table->selectRow(0);
    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        dialog->findChild<QLineEdit*>()->setText("Phim sau cập nhật");
        dialog->accept();
    });
    button(movies, "Sửa")->click();
    if (context.movies.findMovieById("MOV0001")->getTitle() != "Phim sau cập nhật") {
        return false;
    }
    table->selectRow(1);
    confirmDeletion(QMessageBox::Yes);
    button(movies, "Xóa")->click();
    if (table->rowCount() != 3) {
        return false;
    }
    qobject_cast<QPushButton*>(table->cellWidget(1, table->columnCount()-1))->click();
    auto* pages = window.findChild<QStackedWidget*>("pages");
    if (pages->currentWidget()->findChild<QComboBox*>("movieFilter")->currentData() != "MOV0002") {
        return false;
    }
    movies = page(window, "MOVIE_VIEW");
    table->selectRow(2);
    confirmDeletion(QMessageBox::Yes);
    button(movies, "Xóa")->click();
    if (context.movies.findMovieById("MOV0003") || table->rowCount() != 2) {
        return false;
    }
    auto* rooms = page(window, "ROOM_VIEW");
    table = rooms->findChild<QTableWidget*>();
    if (!table || table->rowCount() != 2 || !snapshot(window, "phong")) {
        return false;
    }
    auto* roomSearch = rooms->findChild<QLineEdit*>();
    roomSearch->setText("Sao Mai");
    if (!table->isRowHidden(0) || table->isRowHidden(1)) {
        return false;
    }
    roomSearch->clear();
    table->selectRow(0);
    QTimer::singleShot(0, [] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        dialog->findChild<QLineEdit*>()->setText("Phòng sau cập nhật");
        dialog->accept();
    });
    button(rooms, "Sửa")->click();
    if (context.rooms.findRoomById("ROM0001")->getName() != "Phòng sau cập nhật") {
        return false;
    }
    table->selectRow(0);
    confirmDeletion(QMessageBox::Yes);
    button(rooms, "Xóa")->click();
    if (table->rowCount() != 2) {
        return false;
    }
    qobject_cast<QPushButton*>(table->cellWidget(1, table->columnCount()-1))->click();
    auto* view = pages->currentWidget();
    auto* picker = view->findChild<QComboBox*>();
    auto* grid = view->findChild<QTableView*>("seatMatrix");
    if (!grid || !picker || picker->currentData() != "ROM0002" ||
        grid->model()->rowCount() != 3 || grid->model()->columnCount() != 5) {
        return false;
    }
    picker->setCurrentIndex(picker->findData("ROM0001"));
    QApplication::processEvents();
    if (grid->model()->rowCount() != 5 || grid->model()->columnCount() != 8 ||
        !grid->viewport()->rect().intersects(grid->visualRect(grid->model()->index(0, 7))) || !snapshot(window, "ghe")) {
        return false;
    }
    auto setType = [](QDialog* dialog) {
        dialog->findChild<QComboBox*>()->setCurrentText("COUPLE");
        dialog->accept();
    };
    editSeat(grid, 0, 3, setType);
    const auto* added = context.seats.findSeatById("ROM0001", "A04");
    if (!added || added->getSeatType() != SeatType::COUPLE) {
        return false;
    }
    auto duplicate = std::make_unique<Seat>("ROM0001", "OTHER", 1, 4, SeatType::STANDARD);
    if (context.seats.addSeat(duplicate.get())) {
        duplicate.release();
        return false;
    }
    editSeat(grid, 0, 0, setType);
    if (context.seats.findSeatById("ROM0001", "A01")->getSeatType() != SeatType::COUPLE ||
        context.seats.findSeatById("ROM0002", "A01")->getSeatType() != SeatType::VIP) {
        return false;
    }
    editSeat(grid, 0, 3, [](QDialog* dialog) {
        confirmDeletion(QMessageBox::No);
        button(dialog, "Xóa ghế")->click();
        dialog->reject();
    });
    if (!context.seats.findSeatById("ROM0001", "A04")) {
        return false;
    }
    editSeat(grid, 0, 3, [](QDialog* dialog) {
        confirmDeletion(QMessageBox::Yes);
        button(dialog, "Xóa ghế")->click();
    });
    return !context.seats.findSeatById("ROM0001", "A04") && grid->model()->index(0, 3).data() == "+"
        && testPeople(window, context) && testSchedule(window, context);
}

bool testReadOnlyCatalog(MainWindow& window)
{
    for (const QString permission : {"MOVIE_VIEW", "ROOM_VIEW"}) {
        auto* view = page(window, permission);
        if (!view || button(view, "Sửa") || button(view, "Xóa")) {
            return false;
        }
    }
    auto* view = page(window, "SEAT_VIEW");
    auto* grid = view->findChild<QTableView*>("seatMatrix");
    bool readOnly = false;
    editSeat(grid, 0, 0, [&](QDialog* dialog) {
        readOnly = !dialog->findChild<QComboBox*>()->isEnabled() && !button(dialog, "Xóa ghế");
        dialog->reject();
    });
    auto* members = page(window, "MEMBER_VIEW");
    auto* shows = page(window, "SHOWTIME_VIEW");
    return readOnly && !page(window, "STAFF_VIEW") && members && button(members, "Sửa")
        && shows && !button(shows, "Sửa");
}
