#ifndef PBL2_CORE_DATA_STRUCTURES_DOUBLY_LINKED_LIST_H
#define PBL2_CORE_DATA_STRUCTURES_DOUBLY_LINKED_LIST_H

#include <iostream>
using namespace std;

template <typename T>
struct ListNode
{
    T data;
    ListNode<T>* next;
    ListNode<T>* prev;
    ListNode(const T& x);
};

template <typename T>
class DoublyLinkedList {
private:
    ListNode<T>* head;
public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    void show() const;
    int nodeCount() const;
    void pushFront(const T& x);
    void pushBack(const T& x);
    void insert(const T& x, int k);
    void popFront();
    void popBack();
    void erase(int k);
    void update(int k, const T& x);
    void clear();
    ListNode<T>* search(const T& x);
    ListNode<T>* getAt(int k);
};

//----

template <typename T>
ListNode<T>::ListNode(const T& x) : data(x), next(nullptr), prev(nullptr)
{}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() : head(nullptr)
{}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() {
    clear();
}

template <typename T>
void DoublyLinkedList<T>::show() const {
    ListNode<T>* temp = head;
    while (temp != nullptr) {
        cout << temp->data << " ";
        temp = temp->next;
    }
    cout << endl;
}

template <typename T>
int DoublyLinkedList<T>::nodeCount() const {
    int count = 0;
    ListNode<T>* temp = head;
    while (temp != nullptr) {
        count++;
        temp = temp->next;
    }
    return count;
}

template <typename T>
void DoublyLinkedList<T>::pushFront(const T& x) {
    ListNode<T>* newNode = new ListNode<T>(x);
    if (head == nullptr) {
        head = newNode;
        return;
    }
    newNode->next = head;
    head->prev = newNode;
    head = newNode;
}

template <typename T>
void DoublyLinkedList<T>::pushBack(const T& x) {
    ListNode<T>* newNode = new ListNode<T>(x);
    if (head == nullptr) {
        head = newNode;
        return;
    }
    ListNode<T>* temp = head;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    temp->next = newNode;
    newNode->prev = temp;
}

template <typename T>
void DoublyLinkedList<T>::insert(const T& x, int k) {
    int n = nodeCount();
    if (k < 0 || k > n) {
        return;
    }
    if (k == 0) {
        pushFront(x);
        return;
    }
    if (k == n) {
        pushBack(x);
        return;
    }
    ListNode<T>* temp = getAt(k);
    ListNode<T>* newNode = new ListNode<T>(x);
    newNode->next = temp;
    newNode->prev = temp->prev;
    temp->prev->next = newNode;
    temp->prev = newNode;
}

template <typename T>
void DoublyLinkedList<T>::popFront() {
    if (head == nullptr) {
        return;
    }
    ListNode<T>* temp = head;
    head = head->next;
    if (head != nullptr) {
        head->prev = nullptr;
    }
    delete temp;
}

template <typename T>
void DoublyLinkedList<T>::popBack() {
    if (head == nullptr) {
        return;
    }
    if (head->next == nullptr) {
        popFront();
        return;
    }
    ListNode<T>* temp = head;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    temp->prev->next = nullptr;
    delete temp;
}

template <typename T>
void DoublyLinkedList<T>::erase(int k) {
    int n = nodeCount();
    if (k < 0 || k >= n) {
        return;
    }
    if (k == 0) {
        popFront();
        return;
    }
    if (k == n - 1) {
        popBack();
        return;
    }
    ListNode<T>* temp = getAt(k);
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;
    delete temp;
}

template <typename T>
void DoublyLinkedList<T>::update(int k, const T& x) {
    ListNode<T>* temp = getAt(k);
    if (temp == nullptr) {
        return;
    }
    temp->data = x;
}

template <typename T>
void DoublyLinkedList<T>::clear() {
    while (head != nullptr) {
        popFront();
    }
}

template <typename T>
ListNode<T>* DoublyLinkedList<T>::search(const T& x) {
    ListNode<T>* temp = head;
    while (temp != nullptr) {
        if (temp->data == x) {
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}

template <typename T>
ListNode<T>* DoublyLinkedList<T>::getAt(int k) {
    if (k < 0) {
        return nullptr;
    }
    ListNode<T>* temp = head;
    int index = 0;
    while (temp != nullptr) {
        if (index == k) {
            return temp;
        }
        temp = temp->next;
        index++;
    }
    return nullptr;
}

#endif
