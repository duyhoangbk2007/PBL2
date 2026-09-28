#include <iostream>
#include <string>
using namespace std;

template <typename T>
struct Node
{
    string key;
    T value;
    Node<T>* next;
    Node(const string& key, const T& value);
};

template <typename T>
class HashTable {
private:
    Node<T>** table;
    int capacity;
    int count;
    int hashFunction(const string& key) const;

public:
    HashTable(int capacity);
    ~HashTable();

    bool isEmpty() const;
    int size() const;
    void show() const;
    void insert(const string& key, const T& value);
    void remove(const string& key);
    Node<T>* search(const string& key);
    void clear();
};

//----

template <typename T>
Node<T>::Node(const string& key, const T& value)
    : key(key), value(value), next(nullptr)
{}

template <typename T>
HashTable<T>::HashTable(int capacity)
{
    if (capacity <= 0) {
        capacity = 10;
    }
    this->capacity = capacity;
    this->count = 0;
    this->table = new Node<T>*[this->capacity];
    for (int i = 0; i < this->capacity; i++) {
        this->table[i] = nullptr;
    }
}

template <typename T>
HashTable<T>::~HashTable() {
    this->clear();
    delete[] this->table;
}

template <typename T>
int HashTable<T>::hashFunction(const string& key) const {
    unsigned long long hash = 0;
    for (int i = 0; i < key.length(); i++) {
        hash = hash * 31 + key[i];
    }
    return static_cast<int>(hash % this->capacity);
}

template <typename T>
bool HashTable<T>::isEmpty() const {
    return this->count == 0;
}

template <typename T>
int HashTable<T>::size() const {
    return this->count;
}

template <typename T>
void HashTable<T>::show() const {
    for (int i = 0; i < this->capacity; i++) {
        Node<T>* temp = this->table[i];
        while (temp != nullptr) {
            cout << " -> (" << temp->key << ", " << temp->value << ")";
            temp = temp->next;
        }
        cout << endl;
    }
}

template <typename T>
void HashTable<T>::insert(const string& key, const T& value) {
    int index = this->hashFunction(key);
    Node<T>* temp = this->table[index];
    while (temp != nullptr) {
        if (temp->key == key) {
            temp->value = value;
            return;
        }
        temp = temp->next;
    }
    Node<T>* newNode = new Node<T>(key, value);
    newNode->next = this->table[index];
    this->table[index] = newNode;
    this->count++;
}
template <typename T>
void HashTable<T>::remove(const string& key) {
    int index = this->hashFunction(key);
    Node<T>* temp = this->table[index];
    Node<T>* prev = nullptr;
    while (temp != nullptr) {
        if (temp->key == key) {
            if (prev == nullptr) {
                this->table[index] = temp->next;
            }
            else {
                prev->next = temp->next;
            }
            delete temp;
            this->count--;
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

template <typename T>
Node<T>* HashTable<T>::search(const string& key) {
    int index = this->hashFunction(key);
    Node<T>* temp = this->table[index];
    while (temp != nullptr) {
        if (temp->key == key) {
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}

template <typename T>
void HashTable<T>::clear() {
    for (int i = 0; i < this->capacity; i++) {
        Node<T>* temp = this->table[i];
        while (temp != nullptr) {
            Node<T>* del = temp;
            temp = temp->next;
            delete del;
        }
        this->table[i] = nullptr;
    }
    this->count = 0;
}