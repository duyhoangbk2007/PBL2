#include "LoginView.h"
#include "../business/AuthService.h"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <exception>

LoginView::LoginView(AuthService& auth, const QString& startupError, QWidget* parent)
    : QWidget(parent), auth(auth), username(new QLineEdit), password(new QLineEdit),
      message(new QLabel(startupError)), signIn(new QPushButton("Đăng nhập →"))
{
    setObjectName("loginView");
    setAttribute(Qt::WA_StyledBackground);
    auto* layout = new QVBoxLayout(this);
    auto* card = new QFrame;
    card->setObjectName("loginCard");
    card->setFixedWidth(430);
    layout->addWidget(card, 0, Qt::AlignCenter);
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(36, 36, 36, 36);
    form->setSpacing(10);
    auto* logo = new QLabel("▣");
    logo->setObjectName("logo");
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(38, 38);
    form->addWidget(logo);
    auto* title = new QLabel("PBL2 Cinema");
    title->setObjectName("pageTitle");
    form->addWidget(title);
    form->addWidget(new QLabel("Hệ thống quản lý và bán vé tại quầy"));
    form->addSpacing(12);

    username->setObjectName("username");
    password->setObjectName("password");
    password->setEchoMode(QLineEdit::Password);
    auto* nameLabel = new QLabel("Tên đăng nhập");
    nameLabel->setBuddy(username);
    auto* passLabel = new QLabel("Mật khẩu");
    passLabel->setBuddy(password);
    form->addWidget(nameLabel);
    form->addWidget(username);
    form->addWidget(passLabel);
    form->addWidget(password);
    message->setObjectName("loginError");
    message->setWordWrap(true);
    message->setVisible(!startupError.isEmpty());
    form->addWidget(message);
    form->addSpacing(5);
    signIn->setObjectName("signIn");
    signIn->setEnabled(startupError.isEmpty());
    form->addWidget(signIn);
    connect(signIn, &QPushButton::clicked, this, &LoginView::submit);
    connect(username, &QLineEdit::returnPressed, password, [this] { password->setFocus(); });
    connect(password, &QLineEdit::returnPressed, signIn, &QPushButton::click);
    username->setFocus();
}

void LoginView::reset()
{
    username->clear();
    password->clear();
    message->hide();
    username->setFocus();
}

void LoginView::submit()
{
    if (!signIn->isEnabled()) {
        return;
    }
    message->hide();
    signIn->setEnabled(false);
    signIn->setText("Đang đăng nhập…");
    username->setReadOnly(true);
    password->setReadOnly(true);
    QTimer::singleShot(0, this, [this] {
        bool valid = false;
        QString error = "Tên đăng nhập hoặc mật khẩu không hợp lệ.";
        try {
            valid = auth.login(username->text().toStdString(), password->text().toStdString())
                 && auth.getCurrentUser() != nullptr;
        } catch (const std::exception&) {
            error = "Không thể xác thực. Vui lòng kiểm tra dữ liệu tài khoản.";
        }
        password->clear();
        password->setReadOnly(false);
        username->setReadOnly(false);
        signIn->setText("Đăng nhập →");
        signIn->setEnabled(true);
        if (valid) {
            emit authenticated();
        } else {
            auth.logout();
            message->setText(error);
            message->show();
            password->setFocus();
        }
    });
}
