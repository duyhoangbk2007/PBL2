#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "Repository.h"
#include "../models/User.h"

class UserRepository {
private:
    std::string filePath;
    Repository<User> repo;
    // Chi muc chi muon con tro; repo so huu va giai phong User.
    std::unordered_map<std::string, User*> usersById;
    std::unordered_map<std::string, User*> usersByUsername;

    static std::string getUserId(User* user);
    static User* stringToUser(const std::string& line);
    static std::string userToString(User* user);
    static bool isValidUser(const User* user);

public:
    explicit UserRepository(const std::string& filePath);
    // Khong sao chep repo so huu cac con tro User.
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

    // Nem runtime_error khi file/du lieu loi. Chi load mot lan thanh cong.
    void load();
    void save();
    bool isLoaded();
    int count();
    const User* getAt(int index);
    const User* findById(const std::string& userId) const;
    const User* findByUsername(const std::string& username) const;

    // Chi nhan quyen so huu khi tra ve true. Chua tu ghi file.
    bool add(User* user);
    // Giu nguyen ID, role, dia chi User va cac lien ket Invoice.
    bool update(const User& newValues);
};

#endif
