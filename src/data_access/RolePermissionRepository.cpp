#include "RolePermissionRepository.h"
#include "../utils/FileIO.h"

using namespace std;

RolePermissionRepository::RolePermissionRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool RolePermissionRepository::load()
{
    loaded = false;
    permissions.clear();
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }
    for (const string& line : lines)
    {
        size_t pos = line.find('|');
        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            return false;
        }
        string role = line.substr(0, pos);
        if (role != "ADMIN" && role != "STAFF")
        {
            return false;
        }
        for (const string& item : permissions)
        {
            if (item == line)
            {
                return false;
            }
        }
        permissions.push_back(line);
    }
    loaded = true;
    return true;
}


bool RolePermissionRepository::isLoaded() const
{
    return loaded;
}

bool RolePermissionRepository::hasPermission(const string& role, const string& permission) const
{
    if (!loaded)
    {
        return false;
    }
    string key = role + "|" + permission;
    for (const string& item : permissions)
    {
        if (item == key)
        {
            return true;
        }
    }
    return false;
}
