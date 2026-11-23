#pragma once
#include "CatalogView.h"

class MovieService;
class AuthService;

class MovieManagementView : public CatalogView
{
public:
    MovieManagementView(MovieService& service, AuthService& auth);
    void refresh();
private:
    MovieService& service;
    AuthService& auth;
    void add();
    void detail();
    void edit();
    void remove();
};
