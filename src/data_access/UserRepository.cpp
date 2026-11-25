#include "UserRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>
using namespace std;

// Kiem tra du lieu User
bool UserRepository::isValidUser(const User* user)
{
    if (user == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(user->getUserId(), "USR"))
    {
        return false;
    }
    if (!Validator::isValidText(user->getUsername()) ||
        !Validator::isValidText(user->getPassword()) ||
        !Validator::isValidText(user->getFullName()))
    {
        return false;
    }
    const string& username = user->getUsername();
    if (username.front() == ' ' || username.back() == ' ')
    {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN &&
        user->getRole() != UserRole::STAFF)
    {
        return false;
    }
    return true;
}

UserRepository::UserRepository(const string& filePath)
    : repo(filePath, getUserId, stringToUser, userToString)
{
}

string UserRepository::getUserId(User* user)
{
    if (user == nullptr)
    {
        return "";
    }
    return user->getUserId();
}

User* UserRepository::stringToUser(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong User");
    }
    if (fields[4] != "ADMIN" && fields[4] != "STAFF")
    {
        throw runtime_error("Role User khong hop le");
    }
    UserRole role;
    if (fields[4] == "ADMIN")
    {
        role = UserRole::ADMIN;
    }
    else
    {
        role = UserRole::STAFF;
    }
    if (fields[5] != "0" && fields[5] != "1")
    {
        throw runtime_error("Trang thai User khong hop le");
    }
    bool active = (fields[5] == "1");

    User* user = new User(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        role,
        active
    );
    if (!isValidUser(user))
    {
        delete user;
        throw runtime_error("Du lieu User khong hop le");
    }
    return user;
}

// Chuyen User thanh mot dong TXT
string UserRepository::userToString(User* user)
{
    if (!isValidUser(user))
    {
        throw runtime_error("User khong hop le khi ghi file");
    }
    string role;
    if (user->getRole() == UserRole::ADMIN)
    {
        role = "ADMIN";
    }
    else
    {
        role = "STAFF";
    }
    return user->getUserId() + "|" +
           user->getUsername() + "|" +
           user->getPassword() + "|" +
           user->getFullName() + "|" +
           role + "|" +
           (user->isActive() ? "1" : "0");
}

// Doc du lieu User tu file
bool UserRepository::load()
{
    if (!repo.load())
    {
        return false;
    }

    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            User* user = repo.getAt(i);
            for (int j = i + 1; j < repo.count(); j++)
            {
                User* other = repo.getAt(j);
                if (user->getUsername() == other->getUsername())
                {
                    throw runtime_error("Trung username trong file User");
                }
            }
        }
    }
    catch (...)
    {
        repo.clear();
        throw;
    }
    return true;
}

// Luu du lieu User xuong file
bool UserRepository::save()
{
    return repo.save();
}

// Them User moi
bool UserRepository::add(User* user)
{
    if (!isValidUser(user))
    {
        return false;
    }
    if (findByUsername(user->getUsername()) != nullptr)
    {
        return false;
    }
    return repo.add(user);
}

// Cap nhat thong tin User
bool UserRepository::update(const User& newValues)
{
    if (!repo.isLoaded() || !isValidUser(&newValues))
    {
        return false;
    }

    User* user = repo.findById(newValues.getUserId());
    if (user == nullptr)
    {
        return false;
    }
    if (user->getRole() != newValues.getRole())
    {
        return false;
    }

    User* existingUser = findByUsername(newValues.getUsername());
    if (existingUser != nullptr && existingUser != user)
    {
        return false;
    }

    user->setUsername(newValues.getUsername());
    user->setPassword(newValues.getPassword());
    user->setFullName(newValues.getFullName());
    user->setActive(newValues.isActive());
    return true;
}

bool UserRepository::removeById(const string& userId)
{
    // Chua ho tro xoa tai khoan.
    (void)userId;
    return false;
}

User* UserRepository::findById(const string& userId)
{
    return repo.findById(userId);
}

User* UserRepository::findByUsername(const string& username)
{
    for (int i = 0; i < repo.count(); i++)
    {
        User* user = repo.getAt(i);
        if (user != nullptr && user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

User* UserRepository::getAt(int index)
{
    return repo.getAt(index);
}

int UserRepository::count()
{
    return repo.count();
}

bool UserRepository::isLoaded()
{
    return repo.isLoaded();
}
