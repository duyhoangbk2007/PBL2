#include "AuthService.h"
#include <stdexcept>

using namespace std;

AuthService::AuthService(IRepository<User>& userRepo,
                         RolePermissionRepository& permissionRepo)
    : userRepo(userRepo),
      permissionRepo(permissionRepo)
{
}

const User* AuthService::findByUsername(const string& username) const
{
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

bool AuthService::hasUserPermission(const User& user,
                                    const string& permission) const
{
    if (!permissionRepo.isLoaded())
    {
        return false;
    }
    if (user.getRole() == UserRole::ADMIN)
    {
        return permissionRepo.hasPermission("ADMIN", permission);
    }
    if (user.getRole() == UserRole::STAFF)
    {
        return permissionRepo.hasPermission("STAFF", permission);
    }
    return false;
}

bool AuthService::login(const string& username,
                        const string& password)
{
    logout();
    if (!userRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu tai khoan");
    }
    if (username.empty() || password.empty())
    {
        return false;
    }
    const User* user = findByUsername(username);
    if (user == nullptr || user->getPassword() != password || !user->isActive())
    {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return false;
    }
    currentUserId = user->getUserId();
    return true;
}

void AuthService::logout()
{
    currentUserId.clear();
}

const User* AuthService::getCurrentUser() const
{
    if (currentUserId.empty() || !userRepo.isLoaded())
    {
        return nullptr;
    }
    const User* user = userRepo.findById(currentUserId);
    if (user == nullptr || !user->isActive())
    {
        return nullptr;
    }
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return user;
}

bool AuthService::isLoggedIn() const
{
    return getCurrentUser() != nullptr;
}

bool AuthService::hasPermission(const string& permission) const
{
    const User* user = getCurrentUser();
    return user != nullptr && hasUserPermission(*user, permission);
}

const User& AuthService::requireLogin() const
{
    const User* user = getCurrentUser();
    if (user == nullptr)
    {
        throw runtime_error("Can dang nhap bang tai khoan dang hoat dong");
    }
    return *user;
}

const User& AuthService::requirePermission(const string& permission) const
{
    const User& user = requireLogin();
    if (!hasUserPermission(user, permission))
    {
        throw runtime_error("Khong co quyen: " + permission);
    }
    return user;
}
