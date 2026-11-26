#ifndef ROLE_PERMISSION_REPOSITORY_H
#define ROLE_PERMISSION_REPOSITORY_H

#include <string>
#include <vector>

class RolePermissionRepository
{
private:
    std::string filePath;
    std::vector<std::string> permissions;
    bool loaded;

    static bool isValidRole(const std::string& role);
    static bool isValidPermission(const std::string& permission);

public:
    RolePermissionRepository(const std::string& filePath);

    bool load();

    bool hasPermission(const std::string& role,
                       const std::string& permission) const;

    bool isLoaded() const;
};

#endif
