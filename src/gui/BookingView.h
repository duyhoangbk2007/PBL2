#pragma once
#include <QWidget>
#include <QVector>
#include "../models/Seat.h"
#include "../models/Ticket.h"

class ApplicationContext;
class BookingGrid;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableWidget;

class BookingView : public QWidget
{
    Q_OBJECT
public:
    explicit BookingView(ApplicationContext& context);
    void refresh();
private:
    struct Line { QString id; SeatType seat; TicketType ticket; };
    ApplicationContext& ctx;
    QStackedWidget* steps;
    QLabel *progress, *notice, *reviewTitle, *total, *estimate;
    QLineEdit *search, *memberQuery;
    QComboBox *movies, *shows, *members;
    BookingGrid* grid;
    QTableWidget *tickets, *review;
    QCheckBox *age, *student, *cash;
    QPushButton *back, *next, *undo, *clearSeats, *cancel;
    QVector<Line> choices; // Selection order is also the undo stack.
    QString roomId;
    void populateMovies();
    void populateShows();
    void loadSeats();
    void clearChoices();
    void resetChecks();
    void lookupMember();
    void renderLines();
    bool quote();
    bool validShow() const;
    void setStep(int step);
    void advance();
};
