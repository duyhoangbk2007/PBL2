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
    ShowtimeManagementView(ShowtimeService& service, MovieService& movies, RoomService& rooms, AuthService& auth);
    void refresh(const QString& movieId = {});
private:
    void filter() override;
    void form(bool creating);
    ShowtimeService& service;
    MovieService& movies;
    RoomService& rooms;
    AuthService& auth;
    QComboBox* byMovie;
    QComboBox* byRoom;
    QComboBox* byDate;
    QComboBox* byStatus;
};
