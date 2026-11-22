#include "MainWindow.h"
#include "test_catalog.h"
#include "ApplicationContext.h"

#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QUuid>

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTest>

int testHistory();

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (app.arguments().contains("--history")) {
        return testHistory();
    }
    QTemporaryDir data;
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    QFile users(data.filePath("users.txt"));
    if (!data.isValid() || !users.open(QIODevice::WriteOnly)) {
        return 1;
    }
    users.write("USR0001|manager|" + secret + "|Quan ly rap|ADMIN|1\n"
                "USR0002|cashier|" + secret + "|Nhan vien rap|STAFF|1\n"
                "USR0003|inactive|" + secret + "|Tai khoan khoa|STAFF|0\n"
                "USR0004|second|" + secret + "|Thu ngan|STAFF|1\n");
    users.close();
    ApplicationContext missing(data.path());
    const QString startupError = missing.load();
    MainWindow blocked(missing, startupError);
    if (startupError.isEmpty() || blocked.findChild<QPushButton*>("signIn")->isEnabled()) {
        qCritical("Missing permission data must block sign-in");
        return 1;
    }
    const auto permissionsPath = data.filePath("role_permissions.txt");
    if (!QFile::copy(QString(PBL2_GUI_FIXTURES) + "/role_permissions.txt", permissionsPath)) {
        return 1;
    }
    for (const QString file : {"movies.txt", "rooms.txt", "seats.txt", "showtimes.txt", "members.txt", "pricing.txt"}) {
        if (!QFile::copy(QString(PBL2_GUI_FIXTURES) + "/" + file, data.filePath(file))) {
            return 1;
        }
    }
    ApplicationContext context(data.path());
    const auto error = context.load();
    if (!error.isEmpty() || !context.catalogError.isEmpty()) {
        qCritical("Could not load test accounts");
        return 1;
    }
    MainWindow window(context, error);
    window.show();
    const auto captures = qEnvironmentVariable("PBL2_CATALOG_SCREENSHOTS");
    if (!captures.isEmpty()) {
        QApplication::processEvents();
        if (!window.grab().save(QDir(captures).filePath("dang-nhap.png"))) { return 1; }
    }
    auto signIn = [&](MainWindow& target, const QString& name, const QString& pass) {
        target.findChild<QLineEdit*>("username")->setText(name);
        target.findChild<QLineEdit*>("password")->setText(pass);
        auto* submit = target.findChild<QPushButton*>("signIn");
        submit->click();
        const bool busy = !submit->isEnabled();
        QApplication::processEvents();
        return busy;
    };
    if (window.findChild<QStackedWidget*>("pages") || context.auth.hasPermission("TICKET_SELL") ||
        !window.findChild<QPushButton*>("signIn")->isVisible() ||
        window.findChild<QLineEdit*>("password")->echoMode() != QLineEdit::Password ||
        !signIn(window, "manager", secret + "-incorrect") || context.auth.isLoggedIn() ||
        !window.findChild<QLabel*>("loginError")->isVisible() ||
        !signIn(window, "inactive", secret) || context.auth.isLoggedIn() ||
        !signIn(window, "manager", secret) || !context.auth.isAdmin()) {
        qCritical("Initial login, rejection or admin session failed");
        return 1;
    }
    auto* buttons = window.findChild<QButtonGroup*>();
    auto* pages = window.findChild<QStackedWidget*>("pages");
    auto* crumb = window.findChild<QLabel*>("crumbTitle");
    auto* scroll = window.findChild<QScrollArea*>();
    if (!buttons || !pages || !crumb || !scroll || pages->count() != 11 ||
        buttons->buttons().size() != pages->count() || window.styleSheet().isEmpty()) {
        qCritical("Missing navigation or theme");
        return 1;
    }

    for (const QSize size : {QSize(1280, 820), QSize(1366, 768), QSize(960, 580)}) {
        window.resize(size);
        for (int i = 0; i < pages->count(); ++i) {
            auto* button = buttons->button(i);
            scroll->ensureWidgetVisible(button);
            QApplication::processEvents();
            QTest::mouseClick(button, Qt::LeftButton);
            auto* heading = pages->currentWidget()->findChild<QLabel*>("pageTitle");
            if (window.size() != size || !heading || pages->currentIndex() != i || buttons->checkedId() != i ||
                crumb->text() != button->accessibleName() || heading->text() != crumb->text()) {
                qCritical() << "Navigation failed" << i << pages->currentIndex() << button->accessibleName()
                            << "requested" << size << "actual" << window.size();
                return 1;
            }
        }
    }
    const QString screenshot = qEnvironmentVariable("PBL2_GUI_SCREENSHOT");
    if (!screenshot.isEmpty()) {
        window.resize(1280, 820);
        scroll->ensureWidgetVisible(buttons->button(0));
        buttons->button(0)->click();
        buttons->button(0)->setFocus();
        QApplication::processEvents();
        if (!window.grab().save(screenshot)) {
            qCritical("Could not save screenshot");
            return 1;
        }
    }
    if (!testCatalog(window, context)) {
        qCritical("Catalog interaction failed");
        return 1;
    }
    auto logout = [&] {
        window.findChild<QPushButton*>("logout")->click();
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        return !context.auth.isLoggedIn() && window.findChild<QPushButton*>("signIn")->isVisible()
            && window.findChild<QStackedWidget*>("pages") == nullptr
            && window.findChild<QLineEdit*>("username")->text().isEmpty()
            && window.findChild<QLineEdit*>("password")->text().isEmpty();
    };
    if (!logout() || !signIn(window, "cashier2", secret) || !context.auth.isStaff()) {
        qCritical("Account switch failed");
        return 1;
    }
    buttons = window.findChild<QButtonGroup*>();
    if (buttons->buttons().size() != 8 || buttons->checkedButton()->accessibleName() != "Bán vé tại quầy" ||
        window.findChild<QLabel*>("account")->text() != "Nhan vien moi" ||
        window.findChild<QLabel*>("accountStatus")->text() != "cashier2 · STAFF") {
        qCritical("Staff landing page, header or permission count failed");
        return 1;
    }
    for (auto* button : buttons->buttons()) {
        if (!context.auth.hasPermission(button->property("permission").toString().toStdString())) {
            qCritical("Unauthorized staff menu");
            return 1;
        }
    }
    if (!testReadOnlyCatalog(window) || !logout()) {
        return 1;
    }
    QFile limited(permissionsPath);
    if (!limited.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return 1;
    }
    limited.write("ADMIN|INVOICE_VIEW_ALL\n");
    limited.close();
    ApplicationContext restricted(data.path());
    const QString restrictedError = restricted.load();
    MainWindow limitedWindow(restricted, restrictedError);
    limitedWindow.show();
    signIn(limitedWindow, "manager", secret);
    auto* allowed = limitedWindow.findChild<QButtonGroup*>();
    if (!allowed || allowed->buttons().size() != 1 || allowed->checkedButton()->accessibleName() != "Lịch sử hóa đơn") {
        qCritical("Restricted admin or invoice ALL permission failed");
        return 1;
    }
    limitedWindow.findChild<QPushButton*>("logout")->click();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    signIn(limitedWindow, "cashier", secret);
    allowed = limitedWindow.findChild<QButtonGroup*>();
    if (!restricted.auth.isLoggedIn() || !allowed || !allowed->buttons().isEmpty()) {
        qCritical("Empty permission set granted a page");
        return 1;
    }
    return 0;
}
