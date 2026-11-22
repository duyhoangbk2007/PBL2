#pragma once
#include <QWidget>
#include <QStringList>
#include <functional>

class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QFormLayout;
class QSpinBox;
class QDialog;

namespace CatalogUi {
void run(QLabel* status, const std::function<void()>& action);
bool confirmForm(QDialog& dialog, QFormLayout& form, bool creating);
void result(QLabel* status, bool changed);
QSpinBox* number(QFormLayout& form, const QString& caption, int value, int minimum);
}

class CatalogView : public QWidget
{
    Q_OBJECT
public:
    explicit CatalogView(const QStringList& columns);
signals:
    void openRelated(const QString& id);
protected:
    QLineEdit* search;
    QLabel* status;
    QTableWidget* table;
    QHBoxLayout* toolbar;
    void append(const QStringList& values, const QString& link, const QString& id);
    void action(const QString& caption, bool allowed, const std::function<void()>& handler);
    void pending(const QString& caption, bool allowed, const QString& reason);
    QString selectedId() const;
    virtual void filter();
};
