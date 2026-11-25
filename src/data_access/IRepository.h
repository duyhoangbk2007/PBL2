#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <string>


template <typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;

    // Doc va ghi du lieu
    virtual bool load() = 0;
    virtual bool save() = 0;

    // Quan ly du lieu
    virtual bool add(T* item) = 0;
    virtual bool update(const T& newValues) = 0;
    virtual bool removeById(const std::string& key) = 0;

    // Truy van du lieu
    virtual T* findById(const std::string& key) = 0;
    virtual T* getAt(int index) = 0;
    virtual int count() = 0;
    virtual bool isLoaded() = 0;
};

#endif