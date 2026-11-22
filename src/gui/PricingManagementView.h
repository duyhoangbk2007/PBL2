#pragma once
#include <QWidget>
#include <array>

class AuthService;
class PricingService;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

QString formatVnd(double value);

class PricingManagementView : public QWidget
{
    Q_OBJECT
public:
    PricingManagementView(PricingService& pricing, AuthService& auth);
    void refresh();
private:
    void save();
    PricingService& pricing;
    AuthService& auth;
    std::array<QDoubleSpinBox*, 5> fields{};
    std::array<double, 5> displayed{};
    std::array<QLabel*, 3> cards{};
    QPushButton* apply;
    QLabel* message;
};
