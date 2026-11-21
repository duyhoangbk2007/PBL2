#include "MainWindow.h"
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

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QTemporaryDir data;
    const auto secret = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    QFile users(data.filePath("users.txt"));
    if (!data.isValid() || !users.open(QIODevice::WriteOnly)) {
        return 1;
    }
    users.write("USR0001|manager|" + secret + "|Quan ly rap|ADMIN|1\n"
                "USR0002|cashier|" + secret + "|Nhan vien rap|STAFF|1\n"
                "USR0003|inactive|" + secret + "|Tai khoan khoa|STAFF|0\n");
    users.close();
    ApplicationContext missing(data.path());
    const QString startupError = missing.load();
    MainWindow blocked(missing.auth, startupError);
    if (startupError.isEmpty() || blocked.findChild<QPushButton*>("signIn")->isEnabled()) {
        qCritical("Missing permission data must block sign-in");
        return 1;
    }
    const auto permissionsPath = data.filePath("role_permissions.txt");
    if (!QFile::copy(PBL2_PERMISSIONS_FILE, permissionsPath)) {
        return 1;
    }
    ApplicationContext context(data.path());
    const auto error = context.load();
    if (!error.isEmpty()) {
        qCritical("Could not load test accounts");
        return 1;
    }
    MainWindow window(context.auth, error);
    window.show();
    auto signIn = [&](MainWindow& target, const QString& name, const QString& pass) {
        target.findChild<QLineEdit*>("username")->setText(name);
        target.findChild<QLineEdit*>("password")->setText(pass);
        auto* submit = target.findChild<QPushButton*>("signIn");
        submit->click();
        const bool busy = !submit->isEnabled();
        QApplication::processEvents();
        return busy;
    };
    if (!window.findChild<QPushButton*>("signIn")->isVisible() ||
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

    for (const QSize size : {QSize(1280, 820), QSize(960, 580)}) {
        window.resize(size);
        for (int i = 0; i < pages->count(); ++i) {
            auto* button = buttons->button(i);
            scroll->ensureWidgetVisible(button);
            QApplication::processEvents();
            QTest::mouseClick(button, Qt::LeftButton);
            auto* heading = pages->currentWidget()->findChild<QLabel*>("pageTitle");
            if (!heading || pages->currentIndex() != i || buttons->checkedId() != i ||
                crumb->text() != button->accessibleName() || heading->text() != crumb->text()) {
                qCritical("Navigation failed at page %d", i);
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
    auto logout = [&] {
        window.findChild<QPushButton*>("logout")->click();
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        return !context.auth.isLoggedIn() && window.findChild<QPushButton*>("signIn")->isVisible()
            && window.findChild<QStackedWidget*>("pages") == nullptr
            && window.findChild<QLineEdit*>("username")->text().isEmpty()
            && window.findChild<QLineEdit*>("password")->text().isEmpty();
    };
    if (!logout() || !signIn(window, "cashier", secret) || !context.auth.isStaff()) {
        qCritical("Account switch failed");
        return 1;
    }
    buttons = window.findChild<QButtonGroup*>();
    if (buttons->buttons().size() != 8 || buttons->checkedButton()->accessibleName() != "Bán vé tại quầy" ||
        window.findChild<QLabel*>("account")->text() != "Nhan vien rap" ||
        window.findChild<QLabel*>("accountStatus")->text() != "cashier · STAFF") {
        qCritical("Staff landing page, header or permission count failed");
        return 1;
    }
    for (auto* button : buttons->buttons()) {
        if (!context.auth.hasPermission(button->property("permission").toString().toStdString())) {
            qCritical("Unauthorized staff menu");
            return 1;
        }
    }
    if (!logout()) {
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
    MainWindow limitedWindow(restricted.auth, restrictedError);
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
