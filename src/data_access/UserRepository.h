#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "IRepository.h"
#include "Repository.h"
#include "../models/User.h"


class UserRepository : public IRepository<User> {
private:
    Repository<User> repo;

    std::unordered_map<std::string, User*> usersById; // tìm user bằng id
    std::unordered_map<std::string, User*> usersByUsername; //tìm user bằng tên đăng nhập

    static std::string getUserId(User* user);
    static User* stringToUser(const std::string& line);
    static std::string userToString(User* user);

    // Kiem tra du lieu User
    static bool isValidUser(const User* user);

public:
    UserRepository(const std::string& filePath);

    bool load() override;
    bool save() override;
    
    // Quan ly tai khoan
    bool add(User* user) override;
    bool update(const User& newValues) override;
    bool removeById(const std::string& userId) override;

    // Truy van tai khoan
    User* findById(const std::string& userId) override;
    User* findByUsername(const std::string& username);
    User* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
