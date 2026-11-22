#include "ApplicationContext.h"
#include "BookingAdapter.h"
#include "MainWindow.h"
#include "PricingManagementView.h"
#include <QApplication>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>
#include <stdexcept>

namespace {
void require(bool ok, const char* message)
{
    if (!ok) { throw std::runtime_error(message); }
}

void writeFile(const QString& path, const QByteArray& data)
{
    QFile file(path);
    require(file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(data) == data.size(), "test fixture write");
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    require(file.open(QIODevice::ReadOnly), "test file read");
    return file.readAll();
}

QPushButton* button(QWidget* view, const QString& text)
{
    for (auto* item : view->findChildren<QPushButton*>()) {
        if (item->text() == text) { return item; }
    }
    throw std::runtime_error("missing button");
}

QWidget* navigate(MainWindow& window, const QString& permission)
{
    for (auto* item : window.findChild<QButtonGroup*>()->buttons()) {
        if (item->property("permission") == permission) {
            item->click();
            QApplication::processEvents();
            return window.findChild<QStackedWidget*>("pages")->currentWidget();
        }
    }
    throw std::runtime_error("missing permission");
}

void capture(MainWindow& window, const QString& name)
{
    const auto path = qEnvironmentVariable("PBL2_CATALOG_SCREENSHOTS");
    if (!path.isEmpty()) {
        require(window.grab().save(QDir(path).filePath(name + ".png")), "screenshot");
    }
}

void choose(QComboBox* combo, const QString& id)
{
    int index = combo->findData(id);
    require(index >= 0, "missing option");
    combo->setCurrentIndex(index);
}

void login(MainWindow& window, const QString& username, const QString& secret)
{
    window.findChild<QLineEdit*>("username")->setText(username);
    window.findChild<QLineEdit*>("password")->setText(secret);
    window.findChild<QPushButton*>("signIn")->click();
    QApplication::processEvents();
}

void run()
{
    QTemporaryDir files;
    require(files.isValid(), "temporary directory");
    for (const QString file : {"rooms.txt", "seats.txt", "members.txt", "role_permissions.txt", "pricing.txt"}) {
        require(QFile::copy(QString(PBL2_GUI_FIXTURES) + "/" + file, files.filePath(file)), "copy test fixture");
    }
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    writeFile(files.filePath("users.txt"), "USR0001|manager|" + secret + "|Quan ly|ADMIN|1\n"
              "USR0002|cashier|" + secret + "|Nhan vien|STAFF|1\n");
    const auto future = QDate::currentDate().addDays(5).toString(Qt::ISODate).toUtf8();
    const auto past = QDate::currentDate().addDays(-5).toString(Qt::ISODate).toUtf8();
    writeFile(files.filePath("movies.txt"),
        "MOV0001|Chuyen tau dem|Drama|90|13|2000-01-01|2099-12-31|1\n"
        "MOV0002|Duong chan troi|Drama|90|6|2000-01-01|2099-12-31|1\n");
    auto show = [](QByteArray id, QByteArray movie, QByteArray room, QByteArray date, QByteArray start, QByteArray end, QByteArray state) {
        return id + "|" + movie + "|" + room + "|" + date + "T" + start + "|" + date + "T" + end + "|" + state + "\n";
    };
    writeFile(files.filePath("showtimes.txt"),
        show("SHW0001","MOV0001","ROM0001",future,"09:00:00","10:30:00","SCHEDULED") +
        show("SHW0002","MOV0001","ROM0002",future,"13:00:00","14:30:00","SCHEDULED") +
        show("SHW0003","MOV0002","ROM0002",future,"09:00:00","10:30:00","SCHEDULED") +
        show("SHW0004","MOV0001","ROM0001",past,"09:00:00","10:30:00","SCHEDULED") +
        show("SHW0005","MOV0001","ROM0001",future,"17:00:00","18:30:00","CANCELLED"));
    QFile extraMembers(files.filePath("members.txt"));
    require(extraMembers.open(QIODevice::Append), "member search fixture");
    for (int n = 10; n < 20; ++n) {
        extraMembers.write(QString("MEM00%1|Thu nghiem %1|09000000%1|0\n").arg(n).toUtf8());
    }
    extraMembers.close();
    const auto memberData = readFile(files.filePath("members.txt"));
    ApplicationContext ctx(files.path());
    const auto error = ctx.load();
    require(error.isEmpty() && ctx.pricingError.isEmpty() && ctx.catalogError.isEmpty(), "startup");
    MainWindow window(ctx, error);
    window.show();
    login(window, "manager", secret);
    auto* prices = navigate(window, "PRICING_VIEW");
    auto fields = prices->findChildren<QDoubleSpinBox*>();
    require(fields.size() == 5 && fields[0]->minimum() > 0 && fields[4]->maximum() == 100, "price editor bounds");
    require(!ctx.pricing.updateSeatPrice(SeatType::STANDARD, 0) &&
            !ctx.pricing.updateDiscountRate(DiscountType::MEMBER, 101), "invalid price accepted");
    fields[0]->setValue(89000.5);
    fields[2]->setValue(155000);
    fields[3]->setValue(12);
    button(prices, "Lưu thay đổi")->click();
    require(ctx.pricing.getPricing()->getStudentDiscount() == 12, "discount not updated");
    fields[3]->setValue(10);
    button(prices, "Lưu thay đổi")->click();
    const auto stored = readFile(files.filePath("pricing.txt"));
    require(stored.contains("STANDARD|89000.500000") && stored.contains("COUPLE|155000.") &&
            stored.contains("STUDENT_DISCOUNT|10."), "prices not persisted");
    const auto path = files.filePath("pricing.txt");
    const auto backup = files.filePath("held-prices.txt");
    require(QFile::rename(path, backup) && QDir().mkdir(path), "prepare simulated write failure");
    fields[0]->setValue(88000);
    button(prices, "Lưu thay đổi")->click();
    require(ctx.pricing.getBasePrice(SeatType::STANDARD) == 89000.5 && fields[0]->value() == 89000.5,
            "failed write changed displayed or in-memory price");
    require(QDir().rmdir(path) && QFile::rename(backup, path), "restore temporary pricing file");
    button(prices, "Khôi phục biểu mẫu")->click();
    capture(window, "gia-ve");
    auto* booking = navigate(window, "TICKET_SELL");
    auto* movies = booking->findChild<QComboBox*>("bookingMovie");
    auto* shows = booking->findChild<QComboBox*>("bookingShow");
    auto* grid = booking->findChild<QTableView*>("bookingGrid");
    auto* types = booking->findChild<QTableWidget*>("ticketTypes");
    auto* quote = booking->findChild<QTableWidget*>("quoteLines");
    auto* member = booking->findChild<QComboBox*>("bookingMember");
    auto* query = booking->findChild<QLineEdit*>("memberLookup");
    auto* total = booking->findChild<QLabel*>("quoteTotal");
    auto proceed = [&] { button(booking, "Tiếp tục →")->click(); QApplication::processEvents(); };
    auto cell = [&](int row, int col) {
        auto index = grid->model()->index(row, col);
        grid->scrollTo(index);
        QTest::mouseClick(grid->viewport(), Qt::LeftButton, Qt::NoModifier, grid->visualRect(index).center());
    };
    choose(movies, "MOV0001");
    require(shows->count() == 3 && shows->findData("SHW0004") < 0 && shows->findData("SHW0005") < 0 &&
            shows->findData("SHW0003") < 0, "ineligible or unrelated show");
    choose(shows, "SHW0001");
    proceed();
    require(grid->model()->rowCount() == 5 && grid->model()->columnCount() == 8, "room dimensions");
    require(BookingAdapter::seatStatus("SHW0001","A01") == BookingAdapter::SeatStatus::Unknown &&
            grid->model()->index(0,0).data(Qt::ToolTipRole).toString().contains("Chưa xác minh"), "unknown availability");
    cell(0, 6);
    require(types->rowCount() == 0, "selected missing seat");
    cell(0, 0);
    cell(4, 3);
    button(booking, "Hoàn tác chọn ghế")->click();
    require(types->rowCount() == 1 && types->item(0,0)->text() == "A01", "undo order");
    cell(4, 3);
    cell(4, 3);
    require(types->rowCount() == 2, "couple duplicated");
    auto* clearSeats = button(booking, "Bỏ chọn tất cả ghế");
    clearSeats->click();
    require(!clearSeats->isEnabled() && !button(booking, "Hoàn tác chọn ghế")->isEnabled() &&
            !button(booking, "Tiếp tục →")->isEnabled() && types->rowCount() == 0 && quote->rowCount() == 0 &&
            movies->currentData() == "MOV0001" && shows->currentData() == "SHW0001", "clear seats only");
    cell(0,0);
    cell(4,3);
    require(types->rowCount() == 2 && clearSeats->isEnabled(), "reselect after clear");
    window.resize(1600, 900);
    QApplication::processEvents();
    auto centered = [&] {
        const auto* screen = booking->findChild<QLabel*>("cinemaScreen");
        const int midpoint = screen->mapToGlobal(screen->rect().center()).x();
        const auto first = grid->visualRect(grid->model()->index(0,0));
        const auto last = grid->visualRect(grid->model()->index(0,grid->model()->columnCount()-1));
        const int center = grid->viewport()->mapToGlobal(first.united(last).center()).x();
        return qAbs(midpoint - center) <= grid->verticalHeader()->width() / 2 + 3;
    };
    require(centered(), "wide room must be centered under screen");
    capture(window, "ban-ve-ghe");
    proceed();
    qobject_cast<QComboBox*>(types->cellWidget(1,2))->setCurrentIndex(1);
    query->setText("Thu nghiem");
    require(member->count() == 9, "maximum eight member matches plus guest");
    for (const QString term : {"MEM0001", "0900000001", "Minh Anh"}) {
        query->setText(term);
        require(member->count() == 2 && member->findData("MEM0001") == 1, "member lookup");
    }
    choose(member, "MEM0001");
    proceed();
    require(quote->rowCount() == 2 && quote->item(1,1)->text() == "COUPLE" &&
            quote->item(1,4)->text() == formatVnd(15500) && quote->item(0,4)->text() == formatVnd(4450.025) &&
            quote->item(0,6)->text() == "8" && quote->item(1,6)->text() == "13" &&
            total->text().contains(formatVnd(224050.475)) && total->text().contains("21 điểm"), "per-ticket quote");
    bool coupleProof = false;
    for (auto* check : booking->findChildren<QCheckBox*>()) {
        check->setChecked(true);
        coupleProof |= check->isVisible() && check->text().contains("hai người");
    }
    require(coupleProof && !button(booking, "Hoàn tất giao dịch")->isEnabled(), "checkout guard");
    capture(window, "ban-ve-xem-lai");
    prices = navigate(window, "PRICING_VIEW");
    fields[0]->setValue(90000);
    button(prices, "Lưu thay đổi")->click();
    navigate(window, "TICKET_SELL");
    require(quote->rowCount() == 2 && quote->item(0,3)->text() == formatVnd(90000), "stale quote");
    choose(shows, "SHW0002");
    require(grid->model()->rowCount() == 3 && grid->model()->columnCount() == 5 && types->rowCount() == 0 &&
            !button(booking, "Hoàn tác chọn ghế")->isEnabled() && member->currentIndex() == 0, "show reset");
    proceed();
    cell(0,0);
    require(centered(), "smaller room must recenter after room switch");
    choose(movies, "MOV0002");
    require(shows->count() == 2 && shows->currentIndex() == 0 && grid->model()->rowCount() == 0 &&
            types->rowCount() == 0 && quote->rowCount() == 0 && query->text().isEmpty(), "movie reset");
    for (auto* check : booking->findChildren<QCheckBox*>()) { require(!check->isChecked(), "stale confirmation"); }
    choose(shows, "SHW0003");
    proceed();
    cell(0,0);
    button(booking, "Bỏ giao dịch")->click();
    require(movies->currentIndex() == 0 && types->rowCount() == 0 && quote->rowCount() == 0, "discard");
    require(!QFile::exists(files.filePath("tickets.txt")) && !QFile::exists(files.filePath("invoices.txt")) &&
            memberData == readFile(files.filePath("members.txt")) && ctx.members.findMemberById("MEM0001")->getPoints() == 18,
            "preview affected sales data");
    window.findChild<QPushButton*>("logout")->click();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    login(window, "cashier", secret);
    prices = navigate(window, "PRICING_VIEW");
    for (auto* field : prices->findChildren<QDoubleSpinBox*>()) { require(!field->isEnabled(), "staff price editor enabled"); }
    require(!button(prices, "Lưu thay đổi")->isVisible() && navigate(window,"TICKET_SELL"), "staff permissions");
    const auto before = readFile(files.filePath("pricing.txt"));
    bool denied = false;
    try { ctx.pricing.updateSeatPrice(SeatType::STANDARD, 1000); }
    catch (const std::exception&) { denied = true; }
    require(denied && before == readFile(files.filePath("pricing.txt")), "staff write reached disk");
}

void restartChild(const QString& mode)
{
    const auto secret = qEnvironmentVariable("PBL2_RESTART_TEST_SECRET");
    require(!secret.isEmpty(), "missing isolated test session");
    ApplicationContext ctx(".");
    const auto error = ctx.load();
    require(error.isEmpty() && ctx.catalogError.isEmpty(), "fresh context load");
    MainWindow window(ctx, error);
    window.show();
    login(window, "restart", secret);
    auto* prices = navigate(window, "PRICING_VIEW");
    const auto editors = prices->findChildren<QDoubleSpinBox*>();
    if (mode == "--bad-price") {
        require(!ctx.pricingError.isEmpty() && editors.isEmpty(), "corrupt pricing allowed edits");
        return;
    }
    require(editors.size() == 5 && ctx.pricingError.isEmpty(), "pricing form unavailable");
    if (mode == "--precise-price") {
        require(ctx.pricing.getBasePrice(SeatType::STANDARD) == 0.0000001 &&
                !button(prices, "Lưu thay đổi")->isEnabled(), "lossy rewrite allowed");
        button(prices, "Lưu thay đổi")->click();
        return;
    }
    if (mode == "--write-price") {
        editors[0]->setValue(123456789012.5);
        editors[3]->setValue(17.125);
        button(prices, "Lưu thay đổi")->click();
    }
    require(ctx.pricing.getBasePrice(SeatType::STANDARD) == 123456789012.5 &&
            editors[0]->value() == 123456789012.5 && editors[3]->value() == 17.125, "restart price mismatch");
    window.close();
}

void testRestart()
{
    QTemporaryDir temp;
    const auto folder = temp.filePath("Rạp Kiệt thử nghiệm");
    require(temp.isValid() && QDir().mkpath(folder), "temporary Unicode directory");
    const QDir target(folder);
    const QDir fixtures(PBL2_GUI_FIXTURES);
    const auto names = fixtures.entryList({"*.txt"}, QDir::Files);
    for (const auto& name : names) {
        require(QFile::copy(fixtures.filePath(name), target.filePath(name)), "copy restart fixture");
    }
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    writeFile(target.filePath("users.txt"), "USR0001|restart|" + secret + "|Quan ly|ADMIN|1\n");
    auto child = [&](const QString& mode) {
        QProcess process;
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("PBL2_RESTART_TEST_SECRET", QString::fromLatin1(secret));
        process.setProcessEnvironment(env);
        process.setWorkingDirectory(folder);
        process.setProcessChannelMode(QProcess::ForwardedChannels);
        process.start(QCoreApplication::applicationFilePath(), {"-platform", QApplication::platformName(), mode});
        require(process.waitForFinished(20000) && process.exitStatus() == QProcess::NormalExit &&
                process.exitCode() == 0, "restart process failed");
    };
    child("--write-price");
    const auto path = target.filePath("pricing.txt");
    const auto saved = readFile(path);
    require(saved.trimmed().split('\n').size() == 5 && saved.contains("STANDARD|123456789012.500000") &&
            saved.contains("STUDENT_DISCOUNT|17.125000"), "incomplete persistent pricing");
    child("--read-price");
    require(readFile(path) == saved, "read-only restart changed prices");
    for (const auto& name : names) {
        if (name != "pricing.txt" && name != "users.txt") {
            require(readFile(target.filePath(name)) == readFile(fixtures.filePath(name)), "unrelated file changed");
        }
    }
    const QByteArray damaged = "STANDARD|70000\nSTANDARD|9\n";
    auto precise = saved;
    precise.replace("STANDARD|123456789012.500000", "STANDARD|0.0000001");
    writeFile(path, precise);
    child("--precise-price");
    require(readFile(path) == precise, "high-precision source overwritten");
    writeFile(path, damaged);
    child("--bad-price");
    require(readFile(path) == damaged, "damaged prices overwritten");
}
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    try {
        const auto args = app.arguments();
        if (args.contains("--restart")) {
            testRestart();
        } else if (args.contains("--write-price") || args.contains("--read-price") ||
                   args.contains("--bad-price") || args.contains("--precise-price")) {
            restartChild(args.last());
        } else {
            run();
        }
    }
    catch (const std::exception& error) {
        qCritical("Booking GUI test failed: %s", error.what());
        return 1;
    }
    return 0;
}
