#include "StaffService.h"
#include "../utils/IdGenerator.h"
#include <vector>
using namespace std;

StaffService::StaffService(IRepository<User>& userRepo,
                           AuthService& authService)
    : userRepo(userRepo), authService(authService)
{
}

bool StaffService::load()
{
    return userRepo.load();
}

bool StaffService::save()
{
    return userRepo.save();
}

bool StaffService::addStaff(User* staff)
{
    authService.requirePermission("STAFF_ADD");

    if (!userRepo.isLoaded() ||
        staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }

    vector<string> existingIds;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* current = userRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getUserId());
    }

    string userId = IdGenerator::generateNextId("USR", existingIds);
    User* newStaff = new User(userId,
                              staff->getUsername(),
                              staff->getPassword(),
                              staff->getFullName(),
                              UserRole::STAFF,
                              staff->isActive());

    try
    {
        if (!userRepo.add(newStaff))
        {
            delete newStaff;
            return false;
        }
    }
    catch (...)
    {
        delete newStaff;
        throw;
    }
    return true;
}

bool StaffService::updateStaff(const User& newValues)
{
    authService.requirePermission("STAFF_UPDATE");

    if (!userRepo.isLoaded())
    {
        return false;
    }

    const User* current = userRepo.findById(newValues.getUserId());
    if (current == nullptr ||
        current->getRole() != UserRole::STAFF ||
        newValues.getRole() != UserRole::STAFF)
    {
        return false;
    }

    if (current->isActive() != newValues.isActive())
    {
        return false;
    }
    return userRepo.update(newValues);
}

bool StaffService::setStaffActive(const string& staffId, bool active)
{
    authService.requirePermission("STAFF_DEACTIVATE");

    if (!userRepo.isLoaded())
    {
        return false;
    }

    const User* staff = userRepo.findById(staffId);
    if (staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }

    if (staff->isActive() == active)
    {
        return true;
    }

    User updated(staff->getUserId(),
                 staff->getUsername(),
                 staff->getPassword(),
                 staff->getFullName(),
                 staff->getRole(),
                 active);
    return userRepo.update(updated);
}

const User* StaffService::findStaffById(const string& staffId)
{
    authService.requirePermission("STAFF_VIEW");

    const User* staff = userRepo.findById(staffId);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}

const User* StaffService::findStaffByUsername(const string& username)
{
    authService.requirePermission("STAFF_VIEW");

    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr &&
            user->getRole() == UserRole::STAFF &&
            user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

const User* StaffService::getStaffAt(int index)
{
    authService.requirePermission("STAFF_VIEW");

    if (index < 0)
    {
        return nullptr;
    }

    int staffIndex = 0;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            if (staffIndex == index)
            {
                return user;
            }
            staffIndex++;
        }
    }
    return nullptr;
}

int StaffService::getStaffCount()
{
    authService.requirePermission("STAFF_VIEW");

    int count = 0;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            count++;
        }
    }
    return count;
}
