#pragma once

#include <QMainWindow>

class AuthService;
class LoginView;
class QLabel;
class QStackedWidget;

class MainWindow : public QMainWindow
{
public:
    MainWindow(AuthService& auth, const QString& startupError, QWidget* parent = nullptr);

private:
    QWidget* createSidebar();
    QWidget* createHeader();

    void openSession();
    void closeSession();
    AuthService& auth;
    LoginView* login;
    QStackedWidget* screens;
    QStackedWidget* pages = nullptr;
    QLabel* currentTitle;
};
