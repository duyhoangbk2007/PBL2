#include "AuthService.h"
#include "../data_access/UserRepository.h"

#include <stdexcept>

using namespace std;

AuthService::AuthService(UserRepository& userRepository)
    : userRepository(userRepository)
{
}

bool AuthService::login(const string& username, const string& password)
{
    logout();
    if (!userRepository.isLoaded()) {
        throw runtime_error("Chua nap du lieu tai khoan");
    }
    if (username.empty() || password.empty()) return false;

    const User* user = userRepository.findByUsername(username);
    if (user == nullptr || user->getPassword() != password || !user->isActive()) {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF) {
        return false;
    }

    // Luu ID bat bien, khong sao chep Model hay luu mat khau trong phien.
    currentUserId = user->getUserId();
    return true;
}

void AuthService::logout()
{
    currentUserId.clear();
}

const User* AuthService::getCurrentUser() const
{
    if (currentUserId.empty() || !userRepository.isLoaded()) return nullptr;

    const User* user = userRepository.findById(currentUserId);
    // Kiem tra lai trang thai tu repo o moi lan su dung quyen.
    if (user == nullptr || !user->isActive()) return nullptr;
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF) {
        return nullptr;
    }
    return user;
}

bool AuthService::isLoggedIn() const
{
    return getCurrentUser() != nullptr;
}

bool AuthService::isAdmin() const
{
    const User* user = getCurrentUser();
    return user != nullptr && user->getRole() == UserRole::ADMIN;
}

bool AuthService::isStaff() const
{
    const User* user = getCurrentUser();
    return user != nullptr && user->getRole() == UserRole::STAFF;
}

const User& AuthService::requireLogin() const
{
    const User* user = getCurrentUser();
    if (user == nullptr) {
        throw runtime_error("Can dang nhap bang tai khoan dang hoat dong");
    }
    return *user;
}

const User& AuthService::requireAdmin() const
{
    const User& user = requireLogin();
    if (user.getRole() != UserRole::ADMIN) {
        throw runtime_error("Thao tac yeu cau quyen Admin");
    }
    return user;
}
