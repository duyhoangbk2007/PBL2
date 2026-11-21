#include "StaffService.h"
#include "../utils/IdGenerator.h"
#include <vector>
using namespace std;

StaffService::StaffService(UserRepository& staffRepo, AuthService& authService)
    : staffRepo(staffRepo), authService(authService)
{}

bool StaffService::load()
{
    return staffRepo.load();
}

bool StaffService::save()
{
    return staffRepo.save();
}


bool StaffService::addStaff(User* staff)
{
    authService.requirePermission("STAFF_ADD");

    if (!staffRepo.isLoaded() ||
        staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }
    vector<string> existingIds;

    for (int i = 0; i < staffRepo.count(); i++)
    {
        User* current = staffRepo.getAt(i);
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
        if (!staffRepo.add(newStaff))
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
    if (!staffRepo.isLoaded())
    {
        return false;
    }
    User* current = staffRepo.findById(newValues.getUserId());
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
    return staffRepo.update(newValues);
}

bool StaffService::deactivateStaff(const string& staffId)
{
    authService.requirePermission("STAFF_DEACTIVATE");
    if (!staffRepo.isLoaded())
    {
        return false;
    }
    User* staff = staffRepo.findById(staffId);
    if (staff == nullptr ||
        staff->getRole() != UserRole::STAFF ||
        !staff->isActive())
    {
        return false;
    }
    User updated(
        staff->getUserId(),
        staff->getUsername(),
        staff->getPassword(),
        staff->getFullName(),
        staff->getRole(),
        false
    );
    return staffRepo.update(updated);
}

const User* StaffService::findStaffById(const string& staffId)
{
    authService.requirePermission("STAFF_VIEW");
    const User* staff = staffRepo.findById(staffId);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}

const User* StaffService::findStaffByUsername(const string& username)
{
    authService.requirePermission("STAFF_VIEW");
    const User* staff = staffRepo.findByUsername(username);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}

const User* StaffService::getStaffAt(int index)
{
    authService.requirePermission("STAFF_VIEW");
    if (index < 0)
    {
        return nullptr;
    }
    int staffIndex = 0;
    for (int i = 0; i < staffRepo.count(); i++)
    {
        const User* user = staffRepo.getAt(i);
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
    for (int i = 0; i < staffRepo.count(); i++)
    {
        const User* user = staffRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            count++;
        }
    }
    return count;
}
