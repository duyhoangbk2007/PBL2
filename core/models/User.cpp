#include "User.h"
using namespace std;

User::User(const string& userId,
           const string& username,
           const string& password,
           const string& fullName,
           UserRole role,
           bool active)
    : userId(userId),
      username(username),
      password(password),
      fullName(fullName),
      role(role),
      active(active)
{}

const string& User::getUserId() const {
    return this->userId;
}

const string& User::getUsername() const {
    return this->username;
}

const string& User::getPassword() const {
    return this->password;
}

const string& User::getFullName() const {
    return this->fullName;
}

UserRole User::getRole() const {
    return this->role;
}

bool User::isActive() const {
    return this->active;
}

void User::setUsername(const string& username) {
    this->username = username;
}

void User::setPassword(const string& password) {
    this->password = password;
}

void User::setFullName(const string& fullName) {
    this->fullName = fullName;
}

void User::setActive(bool active) {
    this->active = active;
}
