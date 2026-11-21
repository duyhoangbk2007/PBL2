#include "ApplicationContext.h"

#include <QDir>
#include <exception>

ApplicationContext::ApplicationContext(const QString& dataDirectory)
    : users(QDir(dataDirectory).filePath("users.txt").toStdString()),
      permissions(QDir(dataDirectory).filePath("role_permissions.txt").toStdString()),
      auth(users, permissions)
{
}

QString ApplicationContext::load()
{
    QString source = "users.txt";
    try {
        if (users.load()) {
            source = "role_permissions.txt";
            if (permissions.load()) {
                return {};
            }
        }
    } catch (const std::exception&) {
        // Backend errors may contain record details. Show only the file name.
    }
    return QString("Không thể nạp %1. Kiểm tra tệp dữ liệu rồi mở lại ứng dụng.").arg(source);
}
