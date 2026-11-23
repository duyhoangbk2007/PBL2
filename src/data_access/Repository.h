#ifndef REPOSITORY_H
#define REPOSITORY_H
#include <vector>
#include <stdexcept>
#include "../utils/FileIO.h"
#include <string>
#include "../data_structures/DoublyLinkedList.h"

using namespace std;

template <typename T>
class Repository
{
private:
    string filePath;
    DoublyLinkedList<T*> items;
    bool loaded; // đánh dấu repo đã nạp dữ liệu chưa
    
    string (*getId)(T*); //con trỏ hàm - getId là 1 con trỏ trỏ đến hàm nhận tham số T* và trả về string
    T* (*parse)(const string&); //chuyển chuỗi sang đối tượng cụ thể
    string (*serialize)(T*); //chuyển đối tượng sang chuỗi
public:
    Repository(const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*));
    ~Repository();
    bool load();
    bool save();
    bool add(T* item);
    bool removeById(const string& id);
    T* findById(const string& id);
    T* getAt(int index);
    int count();
    bool isLoaded();
    void clear();
    const string& getFilePath() const;
};

//----

template <typename T>
Repository<T>::Repository(const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*))
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
T* Repository<T>::findById(const string& id)
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
    string id = getId(item);
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
bool Repository<T>::removeById(const string& id)
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
    vector<string> lines;
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
                    throw runtime_error("Du lieu khong hop le");
                }
                string id = getId(item);
                if (id.empty())
                {
                    throw runtime_error("ID rong");
                }
                // Kiem tra ID trung trong danh sach tam
                for (int j = 0; j < tempItems.nodeCount(); j++)
                {
                    T* current = tempItems.getAt(j)->data;
                    if (getId(current) == id)
                    {
                        throw runtime_error("Trung ID");
                    }
                }
                tempItems.pushBack(item);
                // Tu day tempItems quan ly item
                item = nullptr;
            }
            catch (const exception& e)
            {
                // giải phóng đối tượng bị lỗi
                delete item;
                throw runtime_error(
                    "Loi doc file " + filePath +
                    " tai dong " + to_string(i + 1) +
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
    vector<string> lines;
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        string line = serialize(item);
        lines.push_back(line);
    }
    return FileIO::writeLines(filePath, lines);
}
template <typename T>
const string& Repository<T>::getFilePath() const
{
    return filePath;
}

#endif