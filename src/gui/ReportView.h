#pragma once
#include <QWidget>
#include <array>
class AuthService;
class QLabel;
class QBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QTableWidget;
class QTimer;
namespace HistoryUi { class Dates; }

class ReportView : public QWidget
{
public:
    explicit ReportView(AuthService& auth);
private:
    void resizeEvent(QResizeEvent* event) override;
    void load();
    void display();
    AuthService& auth;
    HistoryUi::Dates* dates;
    QCheckBox* preview;
    QLabel* status;
    std::array<QLabel*, 5> metrics;
    QTableWidget *daily, *movies;
    QTimer* timer;
    QGridLayout* cards;
    QBoxLayout* charts;
};
