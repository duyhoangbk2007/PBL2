#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "../data_access/UserRepository.h"

using namespace std;

class AuthService
{
private:
    UserRepository& userRepo;
    string currentUserId; //lưu id tài khoản đăng nhập, khi đăng xuất sẽ xóa chuỗi

public:
    AuthService(UserRepository& userRepo);

    // Dang nhap, dang xuat
    bool login(const string& username, const string& password);
    void logout();

    // Thong tin tai khoan hien tai
    const User* getCurrentUser() const;  //lấy thông tin tài khoản đang đăng nhập vào hệ thống
    bool isLoggedIn() const;

    // Kiem tra quyen
    bool isAdmin() const;
    bool isStaff() const;

    // Yeu cau quyen
    const User& requireLogin() const;
    const User& requireAdmin() const;
};

#endif
