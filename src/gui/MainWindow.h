#pragma once

#include <QMainWindow>

class AuthService;
class ApplicationContext;
class LoginView;
class QLabel;
class QStackedWidget;

class MainWindow : public QMainWindow
{
public:
    MainWindow(ApplicationContext& context, const QString& startupError, QWidget* parent = nullptr);

private:
    QWidget* createSidebar();
    QWidget* createHeader();

    void openSession();
    void closeSession();
    ApplicationContext& context;
    AuthService& auth;
    LoginView* login;
    QStackedWidget* screens;
    QStackedWidget* pages = nullptr;
    QLabel* currentTitle;
};
