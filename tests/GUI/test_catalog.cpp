#include "test_catalog.h"
#include "MainWindow.h"
#include "ApplicationContext.h"

#include <QApplication>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
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
    if (pages->currentWidget()->property("movieFilter") != "MOV0002") {
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
    if (grid->model()->rowCount() != 5 || grid->model()->columnCount() != 8 || !snapshot(window, "ghe")) {
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
    return !context.seats.findSeatById("ROM0001", "A04") && grid->model()->index(0, 3).data() == "+";
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
    return readOnly;
}
