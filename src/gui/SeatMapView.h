#pragma once
#include <QWidget>

class AuthService;
class RoomService;
class SeatService;
class SeatGrid;
class QComboBox;
class QLabel;

class SeatMapView : public QWidget
{
    Q_OBJECT
public:
    SeatMapView(RoomService& rooms, SeatService& seats, AuthService& auth);
    void refresh(const QString& preferredRoom = {});
private:
    void loadRoom();
    void openCell(int row, int column);
    RoomService& rooms;
    SeatService& seats;
    AuthService& auth;
    QComboBox* picker;
    SeatGrid* grid;
    QLabel* status;
};
