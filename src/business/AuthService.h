#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../data_access/RolePermissionRepository.h"
#include "../models/User.h"

class AuthService
{
private:
    IRepository<User>& userRepo;
    RolePermissionRepository& permissionRepo;
    std::string currentUserId;

    const User* findByUsername(const std::string& username) const;
    bool hasUserPermission(const User& user,
                           const std::string& permission) const;

public:
    AuthService(IRepository<User>& userRepo,
                RolePermissionRepository& permissionRepo);

    bool login(const std::string& username,
               const std::string& password);
    void logout();

    const User* getCurrentUser() const;
    bool isLoggedIn() const;

    bool hasPermission(const std::string& permission) const;
    const User& requireLogin() const;
    const User& requirePermission(const std::string& permission) const;
};

#endif
