#ifndef PBL2_CORE_DATA_STRUCTURES_STACK_H
#define PBL2_CORE_DATA_STRUCTURES_STACK_H

#include <iostream>
using namespace std;

template <typename T>
struct StackNode
{
    T data;
    StackNode<T>* next;
    StackNode(const T& x);
};

template <typename T>
class Stack {
    private:
        StackNode<T> *topNode;
        int capacity;
        int count;

    public:
        Stack(int capacity);
        ~Stack();

        bool isEmpty() const;
        bool isFull() const;
        void show() const;
        int size() const;
        void push(const T& x);
        void pop();
        StackNode<T> *top();
};

//----

template <typename T>
StackNode<T>::StackNode(const T& x) : data(x), next(nullptr)
{}

template <typename T>
Stack<T>::Stack(int capacity) : topNode(nullptr), capacity(capacity < 0 ? 0 : capacity), count(0)
{}

template <typename T>
Stack<T>::~Stack() {
    while (!this->isEmpty()) {
        this->pop();
    }
}

template <typename T>
bool Stack<T>::isEmpty() const {
    return this->count == 0;
}

template <typename T>
bool Stack<T>::isFull() const {
    return this->count == this->capacity;
}

template <typename T>
void Stack<T>::show() const {
    StackNode<T>* temp = this->topNode;
    while (temp != nullptr) {
        cout << temp->data << " ";
        temp = temp->next;
    }
    cout << endl;
}

template <typename T>
int Stack<T>::size() const {
    return this->count;
}

template <typename T>
void Stack<T>::push(const T& x) {
    if (this->isFull())
        return;
    StackNode<T> *newNode = new StackNode<T>(x);
    newNode->next = this->topNode;
    this->topNode = newNode;
    this->count++;
}

template <typename T>
void Stack<T>::pop() {
    if (this->isEmpty())
        return;

    StackNode<T> *temp = this->topNode;
    this->topNode = this->topNode->next;
    delete temp;
    this->count--;
}

template <typename T>
StackNode<T> *Stack<T>::top() {
    return this->topNode;
}

#endif
