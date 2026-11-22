#pragma once
#include "CatalogView.h"

class StaffService;
class AuthService;

class StaffManagementView : public CatalogView
{
    Q_OBJECT
public:
    StaffManagementView(StaffService& service, AuthService& auth);
    void refresh();
private:
    void form(bool creating);
    void filter() override;
    void deactivate();
    StaffService& service;
};
