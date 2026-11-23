#pragma once
#include "CatalogView.h"

class AuthService;
class ShowtimeService;
class MovieService;
class RoomService;
class QComboBox;

class ShowtimeManagementView : public CatalogView
{
    Q_OBJECT
public:
    ShowtimeManagementView(ShowtimeService& service, MovieService& movies, RoomService& rooms,
                           AuthService& auth, bool transactionReady);
    void refresh(const QString& movieId = {});
private:
    void filter() override;
    void form(bool creating);
    void detail();
    ShowtimeService& service;
    MovieService& movies;
    RoomService& rooms;
    AuthService& auth;
    bool transactionReady;
    QComboBox* byMovie;
    QComboBox* byRoom;
    QComboBox* byDate;
    QComboBox* byStatus;
};
