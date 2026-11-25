#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/User.h"

class UserRepository : public IRepository<User>
{
private:
    Repository<User> repo;

    static std::string getUserId(User* user);
    static User* stringToUser(const std::string& line);
    static std::string userToString(User* user);

    static bool isValidUser(const User* user);

public:
    UserRepository(const std::string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(User* user) override;
    bool update(const User& newValues) override;
    bool removeById(const std::string& userId) override;

    // Truy van du lieu
    User* findById(const std::string& userId) override;
    User* findByUsername(const std::string& username);
    User* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
