#include "RolePermissionRepository.h"
#include "../utils/FileIO.h"
#include <stdexcept>

using namespace std;

bool RolePermissionRepository::isValidRole(const string& role)
{
    return role == "ADMIN" || role == "STAFF";
}

bool RolePermissionRepository::isValidPermission(const string& permission)
{
    static const vector<string> validPermissions =
    {
        "MOVIE_VIEW",
        "MOVIE_ADD",
        "MOVIE_UPDATE",
        "MOVIE_DELETE",

        "ROOM_VIEW",
        "ROOM_ADD",
        "ROOM_UPDATE",
        "ROOM_DELETE",

        "SEAT_VIEW",
        "SEAT_ADD",
        "SEAT_UPDATE",
        "SEAT_DELETE",

        "SHOWTIME_VIEW",
        "SHOWTIME_ADD",
        "SHOWTIME_UPDATE",
        "SHOWTIME_CANCEL",
        "SHOWTIME_DELETE",

        "STAFF_VIEW",
        "STAFF_ADD",
        "STAFF_UPDATE",
        "STAFF_DEACTIVATE",

        "MEMBER_VIEW",
        "MEMBER_ADD",
        "MEMBER_UPDATE",

        "TICKET_SELL",
        "TICKET_CANCEL",

        "INVOICE_VIEW_OWN",
        "INVOICE_VIEW_ALL",

        "REPORT_VIEW",

        "PRICING_VIEW",
        "PRICING_UPDATE"
    };

    for (const string& item : validPermissions)
    {
        if (item == permission)
        {
            return true;
        }
    }

    return false;
}

RolePermissionRepository::RolePermissionRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool RolePermissionRepository::load()
{
    // Tu choi su dung bang quyen cu neu nap lai that bai
    loaded = false;
    permissions.clear();

    vector<string> lines;

    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        const string& line = lines[i];
        size_t pos = line.find('|');

        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Sai dinh dang role|permission");
        }

        string role = line.substr(0, pos);
        string permission = line.substr(pos + 1);

        if (!isValidRole(role))
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Role khong hop le");
        }
        if (!isValidPermission(permission))
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Permission khong hop le");
        }

        for (size_t j = 0; j < i; j++)
        {
            if (lines[j] == line)
            {
                throw runtime_error("Loi doc file " + filePath +
                                    " tai dong " + to_string(i + 1) +
                                    ": Trung cap role va permission");
            }
        }
    }

    // Chi cap nhat bang quyen khi toan bo du lieu hop le
    permissions.swap(lines);

    loaded = true;
    return true;
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

bool RolePermissionRepository::isLoaded() const
{
    return loaded;
}
