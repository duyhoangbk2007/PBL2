#ifndef PBL2_CORE_MODELS_USER_H
#define PBL2_CORE_MODELS_USER_H

#include <string>

enum class UserRole {
    ADMIN,
    STAFF
};

class User {
private:
    std::string userId;
    std::string username;
    std::string password;
    std::string fullName;
    UserRole role;
    bool active;

public:
    User(const std::string& userId,
         const std::string& username,
         const std::string& password,
         const std::string& fullName,
         UserRole role,
         bool active);

    const std::string& getUserId() const;
    const std::string& getUsername() const;
    const std::string& getPassword() const;
    const std::string& getFullName() const;
    UserRole getRole() const;
    bool isActive() const;

    void setUsername(const std::string& username);
    void setPassword(const std::string& password);
    void setFullName(const std::string& fullName);
    void setActive(bool active);
};

#endif
