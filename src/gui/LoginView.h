#pragma once

#include <QWidget>

class AuthService;
class QLabel;
class QLineEdit;
class QPushButton;

class LoginView : public QWidget
{
    Q_OBJECT
public:
    LoginView(AuthService& auth, const QString& startupError, QWidget* parent = nullptr);
    void reset();

signals:
    void authenticated();

private:
    void submit();

    AuthService& auth;
    QLineEdit* username;
    QLineEdit* password;
    QLabel* message;
    QPushButton* signIn;
};
