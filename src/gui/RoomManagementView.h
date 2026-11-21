#pragma once
#include "CatalogView.h"

class RoomService;
class AuthService;

class RoomManagementView : public CatalogView
{
public:
    RoomManagementView(RoomService& service, AuthService& auth);
    void refresh();
private:
    RoomService& service;
    AuthService& auth;
    void edit();
    void remove();
};
