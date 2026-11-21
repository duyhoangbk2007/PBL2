#pragma once

#include "../business/AuthService.h"
#include <QString>

class ApplicationContext
{
private:
    UserRepository users;
    RolePermissionRepository permissions;

public:
    explicit ApplicationContext(const QString& dataDirectory);
    QString load();
    AuthService auth;
};
