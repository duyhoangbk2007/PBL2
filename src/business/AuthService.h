#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "../data_access/UserRepository.h"
#include "../data_access/RolePermissionRepository.h"

class AuthService
{
private:
    UserRepository& userRepo;
    std::string currentUserId; //lưu id tài khoản đăng nhập, khi đăng xuất sẽ xóa chuỗi
    RolePermissionRepository& permissionRepo;
public:
    AuthService(UserRepository& userRepo, RolePermissionRepository& permissionRepo);

    bool login(const std::string& username, const std::string& password);
    void logout();

     //lấy thông tin tài khoản đang đăng nhập vào hệ thống
    const User* getCurrentUser() const;  
    bool isLoggedIn() const;

    // Kiem tra quyen
    bool isAdmin() const;
    bool isStaff() const;

    bool hasPermission(const std::string& permission) const;

    // kiểm tra quyền của tài khoản đang đăng nhập
    const User& requireLogin() const;
    const User& requireAdmin() const;
    const User& requirePermission(const std::string& permission) const;
};

#endif
