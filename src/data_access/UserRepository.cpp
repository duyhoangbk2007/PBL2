#include "UserRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>

using namespace std;


static bool isValidUserId(const string& id)
{
    if (id.length() != 7 || id.substr(0, 3) != "USR")
    {
        return false;
    }
    for (int i = 3; i < 7; i++)
    {
        if (id[i] < '0' || id[i] > '9')
        {
            return false;
        }
    }
    return id.substr(3) != "0000";
}

// Kiem tra du lieu User
bool UserRepository::isValidUser(const User* user)
{
    if (user == nullptr)
    {
        return false;
    }
    if (!isValidUserId(user->getUserId()))
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
        role = UserRole::ADMIN; // kiểu enum
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

bool UserRepository::load()
{
    if (repo.isLoaded()) // không load lại khi đã có dữ liệu
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    // Xoa chi muc cu
    usersById.clear();
    usersByUsername.clear();
    // Dua du lieu User vao hai map
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            User* user = repo.getAt(i);
            // Kiem tra trung ID va username
            if (usersById.find(user->getUserId()) != usersById.end() ||
                usersByUsername.find(user->getUsername()) != usersByUsername.end())
            {
                throw runtime_error("Trung ID hoac username tai dong " + to_string(i + 1));
            }

            usersById[user->getUserId()] = user;
            usersByUsername[user->getUsername()] = user;
        }
    }
    catch (...)
    {
        usersById.clear();
        usersByUsername.clear();
        repo.clear();
        throw;
    }
    return true;
}

bool UserRepository::save()
{
    return repo.save();
}

User* UserRepository::findById(const string& userId)
{
    auto it = usersById.find(userId);
    if (it == usersById.end())
    {
        return nullptr;
    }
    return it->second;
}

User* UserRepository::findByUsername(const string& username)
{
    auto it = usersByUsername.find(username);
    if (it == usersByUsername.end())
    {
        return nullptr;
    }
    return it->second;
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

bool UserRepository::add(User* user)
{
    if (!repo.isLoaded() || !isValidUser(user))
    {
        return false;
    }
    // Kiem tra trung ID
    if (findById(user->getUserId()) != nullptr)
    {
        return false;
    }
    // Kiem tra trung username
    if (findByUsername(user->getUsername()) != nullptr)
    {
        return false;
    }
    // Khong them lai con tro da duoc quan ly
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == user)
        {
            return false;
        }
    }
    // Them User vao hai map truoc
    string userId = user->getUserId();
    string username = user->getUsername();

    usersById[userId] = user;

    try
    {
        usersByUsername[username] = user;
    }
    catch (...)
    {
        usersById.erase(userId);
        throw;
    }

    // Them User vao Repository
    try
    {
        if (!repo.add(user))
        {
            usersById.erase(userId);
            usersByUsername.erase(username);
            return false;
        }
    }
    catch (...)
    {
        usersById.erase(userId);
        usersByUsername.erase(username);
        throw;
    }

    return true;
}

bool UserRepository::update(const User& newValues)
{
    if (!repo.isLoaded() || !isValidUser(&newValues))
    {
        return false;
    }

    // Tim User can cap nhat
    User* user = findById(newValues.getUserId());

    if (user == nullptr)
    {
        return false;
    }

    // Khong cho phep thay doi role
    if (user->getRole() != newValues.getRole())
    {
        return false;
    }

    // Kiem tra username moi co bi trung khong
    User* existingUser = findByUsername(newValues.getUsername());

    if (existingUser != nullptr && existingUser != user)
    {
        return false;
    }

    string oldUsername = user->getUsername();
    string newUsername = newValues.getUsername();

    // Neu username thay doi, them khoa moi vao map
    if (oldUsername != newUsername)
    {
        usersByUsername[newUsername] = user;
    }

    // Cap nhat thong tin User
    try
    {
        user->setPassword(newValues.getPassword());
        user->setFullName(newValues.getFullName());
        user->setUsername(newUsername);
        user->setActive(newValues.isActive());
    }
    catch (...)
    {
        if (oldUsername != newUsername)
        {
            usersByUsername.erase(newUsername);
        }

        throw;
    }

    // Xoa username cu khoi map
    if (oldUsername != newUsername)
    {
        usersByUsername.erase(oldUsername);
    }

    return true;
}
