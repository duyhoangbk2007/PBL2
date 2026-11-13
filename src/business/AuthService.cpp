#include "AuthService.h"
#include <stdexcept>

using namespace std;

AuthService::AuthService(UserRepository& userRepo)
    : userRepo(userRepo)
{}

bool AuthService::login(const string& username, const string& password)
{
    // Xoa phien dang nhap cu
    logout();
    // Kiem tra du lieu da load
    if (!userRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu tai khoan");
    }
    // Kiem tra thong tin dang nhap
    if (username.empty() || password.empty())
    {
        return false;
    }
    // Tim tai khoan theo username
    const User* user = userRepo.findByUsername(username);
    if (user == nullptr)
    {
        return false;
    }
    // Kiem tra mat khau
    if (user->getPassword() != password)
    {
        return false;
    }
    // Kiem tra trang thai tai khoan
    if (!user->isActive())
    {
        return false;
    }
    // Kiem tra vai tro
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

const User* AuthService::getCurrentUser() const //lấy thông tin tài khoản đang đăng nhập vào hệ thống
{
    if (currentUserId.empty() || !userRepo.isLoaded())
    {
        return nullptr;
    }
    // Tim tai khoan hien tai
    const User* user = userRepo.findById(currentUserId);
    if (user == nullptr)
    {
        return nullptr;
    }
    // Kiem tra tai khoan con hoat dong
    if (!user->isActive())
    {
        return nullptr;
    }
    // Kiem tra vai tro
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return user;
}

bool AuthService::isLoggedIn() const // kiểm tra hiện tại tài khoản có đăng nhập hợp lệ hay k
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

const User& AuthService::requireLogin() const //yêu cầu đăng nhập để thực hiện nghiệp vụ
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
