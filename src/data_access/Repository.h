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
    T* (*parse)(const string&); //chuyển txt sang đối tượng cụ thể
    string (*serialize)(T*); //chuyển đối tượng sang txt
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
        ListNode<T*>* node = items.getAt(0);
        delete node->data; //giải phóng đối tượng
        items.popFront(); //xóa khỏi danh sách
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
    if (!loaded)
    {
        return false;
    }
    if (item == nullptr)
    {
        return false;
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
bool Repository<T>::load()
{
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }
    // Danh sach tam de tranh mat du lieu cu khi doc loi
    DoublyLinkedList<T*> tempItems;  //
    try
    {
        for (int i = 0; i < (int)lines.size(); i++) //duyệt từng dòng trong lines
        {
            T* item = parse(lines[i]);
            if (item == nullptr)
            {
                throw runtime_error("Du lieu khong hop le tai dong " + to_string(i + 1));
            }
            string id = getId(item);
            if (id.empty())
            {
                delete item;
                throw runtime_error("ID rong tai dong " + to_string(i + 1));
            }
            bool duplicate = false;
            for (int j = 0; j < tempItems.nodeCount(); j++)
            {
                ListNode<T*>* node = tempItems.getAt(j);
                if (getId(node->data) == id)
                {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate)
            {
                delete item;
                throw runtime_error("Trung ID tai dong " + to_string(i + 1));
            }
            tempItems.pushBack(item);
        }
    }
    catch (...)
    {
        while (tempItems.nodeCount() > 0)
        {
            ListNode<T*>* node = tempItems.getAt(0);
            delete node->data;
            tempItems.popFront();
        }
        throw;
    }

    // Chi thay du lieu cu sau khi doc thanh cong
    clear(); //xóa dữ liệu cũ trong items
    while (tempItems.nodeCount() > 0)
    {
        ListNode<T*>* node = tempItems.getAt(0);
        items.pushBack(node->data);
        tempItems.popFront();
    }
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
#endif