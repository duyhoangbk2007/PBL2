#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "IRepository.h"
#include "Repository.h"
#include "../models/User.h"

using namespace std;

class UserRepository : public IRepository<User> {
private:
    Repository<User> repo;

    unordered_map<string, User*> usersById; // tìm user bằng id
    unordered_map<string, User*> usersByUsername; //tìm user bằng tên đăng nhập

    static string getUserId(User* user);
    static User* stringToUser(const string& line);
    static string userToString(User* user);

    // Kiem tra du lieu User
    static bool isValidUser(const User* user);

public:
    UserRepository(const string& filePath);

    bool load() override;
    bool save() override;
    
    // Quan ly tai khoan
    bool add(User* user) override;
    bool update(const User& newValues) override;
    bool removeById(const string& userId) override;

    // Truy van tai khoan
    User* findById(const string& userId) override;
    User* findByUsername(const string& username);
    User* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
