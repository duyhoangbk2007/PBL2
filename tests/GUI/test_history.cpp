#include "ApplicationContext.h"
#include "MainWindow.h"
#include "HistoryWidgets.h"
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
#include <QTemporaryDir>
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
    check(cards.size() == 5 && cards[2]->text() == "—" && banner->text().contains("Đang chờ"), "live report");
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
    try { return exercise(); }
    catch (const std::exception& error) {
        qCritical("%s", error.what());
        return 1;
    }
}
