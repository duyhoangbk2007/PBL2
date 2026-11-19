
#ifndef ROLE_PERMISSION_REPOSITORY_H
#define ROLE_PERMISSION_REPOSITORY_H

#include <string>
#include <vector>

using namespace std;

class RolePermissionRepository
{
private:
    string filePath;
    vector<string> permissions;
    bool loaded = false;

public:
    RolePermissionRepository(const string& filePath);
    bool load();
    bool hasPermission(const string& role,const string& permission) const;
    bool isLoaded() const;
};

#endif
