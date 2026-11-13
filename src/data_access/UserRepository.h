#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "Repository.h"
#include "../models/User.h"

using namespace std;

class UserRepository {
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
    // Khong sao chep repo so huu cac con tro User.

    bool load();
    bool save();
    // Quan ly tai khoan
    bool add(User* user);
    bool update(const User& newValues);

    // Truy van tai khoan
    User* findById(const string& userId);
    User* findByUsername(const string& username);
    User* getAt(int index);
    int count();
    bool isLoaded();
};

#endif
