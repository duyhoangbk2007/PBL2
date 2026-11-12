#ifndef PBL2_AUTH_SERVICE_H
#define PBL2_AUTH_SERVICE_H

#include <string>
#include "../models/User.h"

class UserRepository;

class AuthService {
private:
    UserRepository& userRepository;
    std::string currentUserId;

public:
    // Repository phai song lau hon AuthService.
    explicit AuthService(UserRepository& userRepository);
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;

    // Moi lan thu dang nhap deu xoa phien cu.
    // Tra false neu sai thong tin/inactive; nem loi neu repo chua load.
    bool login(const std::string& username, const std::string& password);
    void logout();

    // Con tro muon tu repo, nullptr neu khong co tai khoan hop le.
    const User* getCurrentUser() const;
    bool isLoggedIn() const;
    bool isAdmin() const;
    bool isStaff() const;

    // Service nghiep vu goi truoc thao tac; nem runtime_error neu bi tu choi.
    // requireLogin chap nhan ca Admin va Staff (vi du nghiep vu ban ve).
    const User& requireLogin() const;
    const User& requireAdmin() const;
};

#endif
