#include "ApplicationContext.h"
#include "MainWindow.h"
#include "HistoryWidgets.h"
#include "InvoiceHistoryView.h"
#include "TicketCancellationView.h"
#include "ReportView.h"
#include "PricingManagementView.h"
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDateEdit>
#include <QDialog>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableView>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUuid>
#include <stdexcept>

namespace {
void check(bool ok, const char* message)
{
    if (!ok) { throw std::runtime_error(message); }
}

QPushButton* button(QWidget* parent, const QString& text)
{
    for (auto* item : parent->findChildren<QPushButton*>()) {
        if (item->text() == text) { return item; }
    }
    throw std::runtime_error("missing button");
}

void exerciseLiveInvoices()
{
    QTemporaryDir data;
    check(data.isValid(), "live history tempdir");
    for (const QString name : {"movies", "rooms", "seats", "pricing", "members", "role_permissions"}) {
        const QString file = name + ".txt";
        check(QFile::copy(QString(PBL2_GUI_FIXTURES) + "/" + file, data.filePath(file)), "live fixture copy");
    }
    auto write = [&](const QString& name, const QByteArray& bytes) {
        QFile file(data.filePath(name));
        check(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "live fixture write");
    };
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    write("users.txt", "USR0001|admin|" + secret + "|Quan ly|ADMIN|1\n"
                       "USR0002|staff|" + secret + "|Nhan vien|STAFF|1\n");
    const auto today = QDate::currentDate().toString(Qt::ISODate).toUtf8();
    const auto yesterday = QDate::currentDate().addDays(-1).toString(Qt::ISODate).toUtf8();
    const auto future = QDate::currentDate().addDays(5).toString(Qt::ISODate).toUtf8();
    write("showtimes.txt", "SHW0001|MOV0002|ROM0002|" + future + "T10:00:00|" + future + "T12:00:00|SCHEDULED\n");
    write("invoices.txt", "INV0001|SHW0001|USR0001||" + today + "T00:00:00|150000\n"
                          "INV0002|SHW0001|USR0002||" + today + "T00:00:00|75000\n"
                          "INV0003|SHW0001|USR0002||" + yesterday + "T08:00:00|75000\n");
    write("tickets.txt", "TKT0001|INV0001|SHW0001|A01|REGULAR|75000|0|75000|0|SOLD||||0\n"
                         "TKT0002|INV0001|SHW0001|A02|REGULAR|75000|0|75000|0|CANCELLED|" + today + "T10:00:00|USR0001|Doi lich|75000\n"
                         "TKT0003|INV0002|SHW0001|C04|REGULAR|75000|0|75000|0|SOLD||||0\n"
                         "TKT0004|INV0003|SHW0001|A02|REGULAR|75000|0|75000|0|SOLD||||0\n");
    ApplicationContext context(data.path());
    check(context.load().isEmpty(), "live context load");
    if (!context.transactionError.isEmpty()) {
        throw std::runtime_error(("live transaction fixture load: " + context.transactionError).toStdString());
    }
    check(context.auth.login("admin", secret.toStdString()), "live admin login");
    InvoiceHistoryView admin(context.invoices, context.auth);
    auto* table = admin.findChild<QTableWidget*>();
    check(table && table->rowCount() == 3 && table->item(0, 11)->text() == "PARTIALLY_CANCELLED" &&
          table->item(0, 8)->text() == formatVnd(150000) &&
          table->item(0, 9)->text() == formatVnd(75000) &&
          table->item(0, 10)->text() == formatVnd(75000), "live invoice amounts and status");
    ReportView report(context.reports, context.auth);
    report.findChild<QDateEdit*>("fromDate")->setDate(QDate::currentDate());
    report.findChild<QDateEdit*>("toDate")->setDate(QDate::currentDate());
    button(&report, "Áp dụng")->click();
    QApplication::processEvents();
    const auto metrics = report.findChildren<QLabel*>("priceValue");
    const auto charts = report.findChildren<QTableWidget*>();
    check(metrics.size() == 5 && metrics[0]->text() == formatVnd(225000) &&
          metrics[1]->text() == formatVnd(75000) &&
          metrics[2]->text() == formatVnd(150000) &&
          metrics[3]->text() == "3" && metrics[4]->text() == "1" &&
          charts.size() == 2 && charts[0]->rowCount() == 1 &&
          charts[1]->rowCount() == 1 && charts[1]->item(0, 2)->text() == formatVnd(150000) &&
          charts[1]->item(0, 3)->text() == "—", "partial refund report totals");
    TicketCancellationView cancellation(context.booking, context.invoices, context.auth);
    auto* search = cancellation.findChild<QLineEdit*>();
    search->setText("TKT0001");
    check(cancellation.findChild<QLabel*>("demoNotice")->text().contains("BLOCKED"), "ticket lookup API gap");
    search->setText("INV9999");
    check(cancellation.findChild<QTableWidget*>()->rowCount() == 0, "missing invoice lookup");
    search->setText("INV0001");
    auto* saleLines = cancellation.findChild<QTableWidget*>();
    check(saleLines->rowCount() == 2 && saleLines->item(0, 5)->text() == "SOLD" &&
          saleLines->item(1, 5)->text() == "CANCELLED" &&
          saleLines->cellWidget(0, 6) && !saleLines->cellWidget(1, 6), "live cancellation rows");
    bool opened = false;
    bool retryReady = false;
    QTimer::singleShot(0, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog) { return; }
        opened = true;
        auto* reason = dialog->findChild<QPlainTextEdit*>();
        auto* submit = button(dialog, "Xác nhận hủy vé");
        reason->setPlainText("Ly do|khong hop le");
        dialog->findChild<QCheckBox*>()->setChecked(true);
        submit->click();
        retryReady = submit->isEnabled();
        reason->setPlainText("Khach doi lich");
        submit->click();
        QTimer::singleShot(0, dialog, &QDialog::reject);
    });
    button(&cancellation, "Hủy vé")->click();
    check(opened && retryReady && context.invoices.getInvoiceStatus("INV0001") == InvoiceStatus::CANCELLED &&
          context.invoices.getRefundAmount("INV0001") == 150000 &&
          !saleLines->cellWidget(0, 6), "cancelTicket did not persist and refresh");
    context.auth.logout();
    check(context.auth.login("staff", secret.toStdString()), "live staff login");
    InvoiceHistoryView staff(context.invoices, context.auth);
    auto* own = staff.findChild<QTableWidget*>();
    check(own && own->rowCount() == 1 && own->item(0, 0)->text() == "INV0002", "staff today scope");
    staff.findChild<QLineEdit*>("invoiceCode")->setText("INV0003");
    button(&staff, "Tra cứu")->click();
    check(own->rowCount() == 1 && own->item(0, 0)->text() == "INV0003", "staff own old invoice lookup");
    staff.findChild<QLineEdit*>("invoiceCode")->setText("INV0001");
    button(&staff, "Tra cứu")->click();
    check(own->rowCount() == 0, "staff accessed admin invoice");
    check(SalesHistory::report(context.reports, context.auth,
          {QDate::currentDate().startOfDay(), QDate::currentDate().addDays(1).startOfDay()}, false)
          .status.state == SalesHistory::State::Error, "staff report access");
    TicketCancellationView denied(context.booking, context.invoices, context.auth);
    denied.findChild<QLineEdit*>()->setText("INV0002");
    check(denied.findChild<QTableWidget*>()->rowCount() == 0, "staff cancellation access");
}

void capture(QWidget& window, const QString& name)
{
    const auto path = qEnvironmentVariable("PBL2_HISTORY_SCREENSHOTS");
    if (!path.isEmpty()) {
        QApplication::processEvents();
        check(window.grab().save(QDir(path).filePath(name + ".png")), "screenshot failed");
    }
}

int exercise()
{
    QTemporaryDir data;
    check(data.isValid(), "tempdir");
    for (const QString name : {"movies", "rooms", "seats", "showtimes", "pricing", "members", "role_permissions"}) {
        const QString file = name + ".txt";
        check(QFile::copy(QString(PBL2_GUI_FIXTURES) + "/" + file, data.filePath(file)), "fixture copy");
    }
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    QFile users(data.filePath("users.txt"));
    check(users.open(QIODevice::WriteOnly), "users fixture");
    users.write("USR0001|admin|" + secret + "|Quan ly|ADMIN|1\nUSR0002|staff|" + secret + "|Nhan vien|STAFF|1\n");
    users.close();
    // Opaque canaries catch accidental writes even when the demo dialog is opened.
    auto contents = [&](const QString& name) {
        QFile file(data.filePath(name));
        check(file.open(QIODevice::ReadOnly), "canary read");
        return file.readAll();
    };
    for (const QString name : {"tickets.txt", "invoices.txt"}) {
        QFile file(data.filePath(name));
        check(file.open(QIODevice::WriteOnly), "canary write");
        file.write("opaque test canary");
    }
    const auto memberBytes = contents("members.txt");
    ApplicationContext context(data.path());
    const auto error = context.load();
    check(error.isEmpty(), "context load");
    MainWindow window(context, error);
    window.resize(1440, 920);
    window.show();
    auto login = [&](const QString& username) {
        window.findChild<QLineEdit*>("username")->setText(username);
        window.findChild<QLineEdit*>("password")->setText(secret);
        window.findChild<QPushButton*>("signIn")->click();
        QApplication::processEvents();
        check(context.auth.isLoggedIn(), "login failed");
    };
    auto page = [&](const QString& permission, const QString& name) -> QWidget* {
        for (auto* item : window.findChild<QButtonGroup*>()->buttons()) {
            if (item->property("permission") == permission) {
                item->click();
                QApplication::processEvents();
                return window.findChild<QStackedWidget*>("pages")->currentWidget()->findChild<QWidget*>(name);
            }
        }
        return nullptr;
    };
    login("admin");
    check(!context.transactionError.isEmpty(), "corrupt transaction fixture");
    auto* seatMap = page("SEAT_VIEW", "seatMap");
    auto* matrix = seatMap ? seatMap->findChild<QTableView*>("seatMatrix") : nullptr;
    check(matrix && matrix->model()->rowCount() > 0, "seat map unavailable with transaction error");
    const auto emptySeat = matrix->model()->index(0, 3);
    matrix->scrollTo(emptySeat);
    QTest::mouseClick(matrix->viewport(), Qt::LeftButton, Qt::NoModifier,
                      matrix->visualRect(emptySeat).center());
    check(!QApplication::activeModalWidget() && !context.seats.findSeatById("ROM0001", "A04"),
          "seat edit allowed with unverified transaction data");
    page("SHOWTIME_VIEW", "");
    auto* shows = window.findChild<QStackedWidget*>("pages")->currentWidget();
    check(shows && shows->findChild<QTableWidget*>()->rowCount() == 3 &&
          !button(shows, "Thêm suất chiếu")->isEnabled() &&
          !button(shows, "Sửa")->isEnabled(), "showtime edits allowed with transaction error");
    auto* history = page("INVOICE_VIEW_OWN", "invoiceHistory");
    check(history, "admin history");
    auto* invoices = history->findChild<QTableWidget*>();
    auto* preview = history->findChild<QCheckBox*>();
    auto* code = history->findChild<QLineEdit*>("invoiceCode");
    check(!preview->isChecked() && invoices->rowCount() == 0 && !button(history, "Xem chi tiết")->isEnabled() &&
          SalesHistory::history(context.auth, {}, false).status.state == SalesHistory::State::NotImplemented, "live history");
    preview->click();
    check(invoices->rowCount() == 3 && invoices->item(0, 4)->text() == "DEMO-S01" &&
          invoices->item(1, 11)->text() == "ACTIVE" && invoices->item(2, 11)->text() == "CANCELLED", "demo invoice shows");
    invoices->selectRow(0);
    bool detailOk = false;
    QTimer::singleShot(0, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog) { return; }
        auto* lines = dialog->findChild<QTableWidget*>();
        detailOk = lines && lines->rowCount() == 2 && lines->item(0,0)->text() == "DEMO-T01"
            && lines->item(1,0)->text() == "DEMO-T02" && lines->item(1,7)->text() == "CANCELLED"
            && !lines->item(1,1)->flags().testFlag(Qt::ItemIsEditable)
            && lines->editTriggers() == QAbstractItemView::NoEditTriggers;
        dialog->reject();
    });
    button(history, "Xem chi tiết")->click();
    check(detailOk, "read-only detail");
    capture(window, "lich-su-hoa-don");
    for (auto* field : history->findChildren<QLineEdit*>()) {
        if (field->accessibleName().isEmpty()) { continue; }
        field->setText("unmatched");
        button(history, "Tra cứu")->click();
        check(invoices->rowCount() == 0, "invoice filter not applied");
        button(history, "Xóa bộ lọc")->click();
        check(invoices->rowCount() == 3, "invoice reset");
    }
    const QDate today = QDate::currentDate();
    HistoryUi::Dates range;
    range.setRange(today, today);
    check(range.period().contains(today.endOfDay()) && !range.period().contains(today.addDays(1).startOfDay()),
          "exclusive end of day");
    history->findChild<QDateEdit*>("toDate")->setDate(today.addDays(-1));
    button(history, "Tra cứu")->click();
    check(invoices->rowCount() == 2 && invoices->item(0,0)->text() == "DEMO-INV02", "history dates");
    preview->click();
    auto* cancellation = page("TICKET_CANCEL", "ticketCancellation");
    check(cancellation, "admin cancellation");
    auto* tickets = cancellation->findChild<QTableWidget*>();
    check(!tickets->rowCount() && SalesHistory::cancellationLookup(context.auth, "", false).status.state ==
          SalesHistory::State::NotImplemented, "live cancellation");
    cancellation->findChild<QCheckBox*>()->click();
    cancellation->findChild<QLineEdit*>()->setText("DEMO-T02");
    check(tickets->rowCount() == 2 && tickets->cellWidget(0, 6) && !tickets->cellWidget(1, 6), "per-ticket action");
    bool locked = false;
    QTimer::singleShot(0, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog) { return; }
        dialog->findChild<QPlainTextEdit*>()->setPlainText("Test");
        dialog->findChild<QCheckBox*>()->click();
        auto* submit = button(dialog, "Xác nhận hủy vé");
        locked = !submit->isEnabled();
        submit->click();
        dialog->reject();
    });
    button(cancellation, "Hủy vé")->click();
    check(locked && tickets->item(0, 5)->text() == "SOLD", "refund guard");
    capture(window, "huy-ve");
    cancellation->findChild<QLineEdit*>()->setText("DEMO-T03");
    check(tickets->rowCount() == 1 && tickets->item(0,5)->text() == "SOLD" &&
          !tickets->cellWidget(0,6), "past show offered cancellation");
    cancellation->findChild<QLineEdit*>()->setText("unknown");
    check(!tickets->rowCount(), "empty lookup");
    auto* reports = page("REPORT_VIEW", "reportView");
    check(reports, "admin report");
    auto* banner = reports->findChild<QLabel*>("demoNotice");
    auto cards = reports->findChildren<QLabel*>("priceValue");
    check(cards.size() == 5 && cards[2]->text() == "—" && banner->text().contains("Chọn ngày"), "live report");
    auto* demo = reports->findChild<QCheckBox*>();
    demo->click();
    check(banner->text().contains("Đang tải"), "report loading");
    QApplication::processEvents();
    check(banner->text().contains("DEMO") && cards[2]->text() == formatVnd(40000), "report preview");
    auto* ranking = reports->findChildren<QTableWidget*>().last();
    const auto* title = ranking->item(0,0);
    check(title && title->text().size() > 50 && title->toolTip() == title->text()
          && ranking->textElideMode() == Qt::ElideRight && !ranking->wordWrap(), "long movie label");
    capture(window, "bao-cao");
    auto* from = reports->findChild<QDateEdit*>("fromDate");
    auto* to = reports->findChild<QDateEdit*>("toDate");
    auto apply = [&] { button(reports, "Áp dụng")->click(); QApplication::processEvents(); };
    from->setDate(today.addDays(-1));
    to->setDate(today.addDays(-1));
    apply();
    check(cards[2]->text() == formatVnd(-60000), "negative cash flow");
    from->setDate(today);
    apply();
    check(cards[2]->text() == "—" && banner->text().contains("Ngày kết thúc"), "invalid range");
    from->setDate(today.addDays(-3));
    to->setDate(today.addDays(-3));
    apply();
    check(banner->text().contains("Không có dữ liệu"), "empty period");
    demo->click();
    QApplication::processEvents();
    check(cards[2]->text() == "—" && !ranking->rowCount(), "leaked preview");
    window.findChild<QPushButton*>("logout")->click();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    login("staff");
    history = page("INVOICE_VIEW_OWN", "invoiceHistory");
    check(history && !page("REPORT_VIEW", "reportView") && !page("TICKET_CANCEL", "ticketCancellation"), "staff navigation");
    code = history->findChild<QLineEdit*>("invoiceCode");
    code->setText("DEMO-INV01");
    button(history, "Tra cứu")->click();
    check(!history->findChild<QCheckBox*>()->isVisible() && !history->findChild<QTableWidget*>()->rowCount() &&
          SalesHistory::history(context.auth, {{}, "DEMO-INV01", {}, {}, {}}, true).invoices.isEmpty(), "staff scope guard");
    check(SalesHistory::report(context.auth, {}, true).status.state == SalesHistory::State::Error &&
          SalesHistory::cancellationLookup(context.auth, {}, true).status.state == SalesHistory::State::Error, "direct access");
    check(contents("tickets.txt") == "opaque test canary" && contents("invoices.txt") == "opaque test canary"
          && contents("members.txt") == memberBytes, "data changed");
    return 0;
}
}

int testHistory()
{
    try {
        exerciseLiveInvoices();
        return exercise();
    }
    catch (const std::exception& error) {
        qCritical("%s", error.what());
        return 1;
    }
}
