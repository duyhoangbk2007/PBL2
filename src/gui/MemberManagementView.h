#pragma once
#include "CatalogView.h"

class MemberService;
class AuthService;

class MemberManagementView : public CatalogView
{
    Q_OBJECT
public:
    MemberManagementView(MemberService& service, AuthService& auth);
    void refresh();
private:
    void form(bool creating);
    void filter() override;
    MemberService& service;
};
