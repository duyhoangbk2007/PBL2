#ifndef REPOSITORY_H
#define REPOSITORY_H
#include <vector>
#include <stdexcept>
#include "../utils/FileIO.h"
#include <string>
#include "../data_structures/DoublyLinkedList.h"


template <typename T>
class Repository
{
private:
    std::string filePath;
    DoublyLinkedList<T*> items;
    bool loaded; // đánh dấu repo đã nạp dữ liệu chưa
    
    std::string (*getId)(T*); //con trỏ hàm - getId là 1 con trỏ trỏ đến hàm nhận tham số T* và trả về string
    T* (*parse)(const std::string&); //chuyển txt sang đối tượng cụ thể
    std::string (*serialize)(T*); //chuyển đối tượng sang txt
public:
    Repository(const std::string& filePath, std::string (*getId)(T*), T* (*parse)(const std::string&), std::string (*serialize)(T*));
    ~Repository();

    bool load();
    bool save();

    bool add(T* item);
    bool removeById(const std::string& id);
    
    T* findById(const std::string& id);
    T* getAt(int index);
    int count();
    bool isLoaded();
    void clear();
    const std::string& getFilePath() const;
};

//----

template <typename T>
Repository<T>::Repository(const std::string& filePath, std::string (*getId)(T*), T* (*parse)(const std::string&), std::string (*serialize)(T*))
    : filePath(filePath), loaded(false), getId(getId), parse(parse), serialize(serialize)
{
}

template <typename T>
Repository<T>::~Repository()
{
    clear();
}

template <typename T>
void Repository<T>::clear()
{
    while (items.nodeCount() > 0)
    {
        T* item = items.getAt(0)->data;
        items.popFront(); //xóa khỏi danh sách
        delete item; //giải phóng đối tượng
    }
    loaded = false;
}
template <typename T>
int Repository<T>::count()
{
    return items.nodeCount();
}
template <typename T>
T* Repository<T>::getAt(int index)
{
    ListNode<T*>* node = items.getAt(index);
    if (node == nullptr)
    {
        return nullptr;
    }
    return node->data;
}
template <typename T>
bool Repository<T>::isLoaded()
{
    return loaded;
}
template <typename T>
T* Repository<T>::findById(const std::string& id)
{
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        if (getId(item) == id)
        {
            return item;
        }
    }
    return nullptr;
}
template <typename T>
bool Repository<T>::add(T* item)
{
    if (!loaded || item == nullptr)
    {
        return false;
    }
    // kiểm tra trùng 
    for (int i = 0; i < count(); i++)
    {
        if (getAt(i) == item)
        {
            return false;
        }
    }
    std::string id = getId(item);
    if (id.empty())
    {
        return false;
    }
    if (findById(id) != nullptr)
    {
        return false;
    }
    items.pushBack(item);
    return true;
}

template <typename T>
bool Repository<T>::removeById(const std::string& id)
{
    if (!loaded)
    {
        return false;
    }
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        if (getId(item) == id)
        {
            items.erase(i);
            delete item;
            return true;
        }
    }
    return false;
}

template <typename T>
bool Repository<T>::load()
{
    if (loaded)
    {
        return false;
    }
    std::vector<std::string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }
    DoublyLinkedList<T*> tempItems;
    try
    {
        for (int i = 0; i < (int)lines.size(); i++)
        {
            T* item = nullptr;
            try
            {
                item = parse(lines[i]);
                if (item == nullptr)
                {
                    throw std::runtime_error("Du lieu khong hop le");
                }
                std::string id = getId(item);
                if (id.empty())
                {
                    throw std::runtime_error("ID rong");
                }
                // Kiem tra ID trung trong danh sach tam
                for (int j = 0; j < tempItems.nodeCount(); j++)
                {
                    T* current = tempItems.getAt(j)->data;
                    if (getId(current) == id)
                    {
                        throw std::runtime_error("Trung ID");
                    }
                }
                tempItems.pushBack(item);
                // Tu day tempItems quan ly item
                item = nullptr;
            }
            catch (const std::exception& e)
            {
                // giải phóng đối tượng bị lỗi
                delete item;
                throw std::runtime_error(
                    "Loi doc file " + filePath +
                    " tai dong " + std::to_string(i + 1) +
                    ": " + e.what()
                );
            }
            catch (...)
            {
                delete item;
                throw;
            }
        }
    }
    catch (...)
    {
        // Giải phóng các đối tượng còn lại
        while (tempItems.nodeCount() > 0)
        {
            T* item = tempItems.getAt(0)->data;
            tempItems.popFront();
            delete item;
        }
        throw;
    }
    items.swap(tempItems);
    loaded = true;
    return true;
}

template <typename T>
bool Repository<T>::save()
{
    if (!loaded)
    {
        return false;
    }
    std::vector<std::string> lines;
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        std::string line = serialize(item);
        lines.push_back(line);
    }
    return FileIO::writeLines(filePath, lines);
}
template <typename T>
const std::string& Repository<T>::getFilePath() const
{
    return filePath;
}

#endif