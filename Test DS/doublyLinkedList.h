#include <iostream>

struct Node {
    int data;
    Node *next;
    Node *prev;
};

Node *makeNode(int x);
void Show(Node *head);
int nodeCount(Node *head);
void pushFront(Node **head, int x);
void pushBack(Node **head, int x);
void insert(Node **head, int x);
void popFront(Node **head);
void popBack(Node **head);
void erase(Node **head, int k);
void clear(Node **head);
Node *search(Node *head, int x);
Node *getAt(Node *head, int k);

Node *makeNode(int x) {
    Node *newNode = new Node;
    newNode->data = x;
    newNode->next = nullptr;
    newNode->prev = nullptr;
    return newNode;
}
void Show(Node *head) {
    while (head != nullptr) {
        std::cout << head->data;
        head = head->next;
    }
}
int nodeCount(Node *head) {
    int count = 0;
    while (head != nullptr) {
        count++;
        head = head->next;
    }
    return count;
}
void pushFront(Node **head, int x) {
    Node *newNode = makeNode(x);
    newNode->next = *head;
    if (*head != nullptr)
        (*head)->prev = newNode;
    *head = newNode;
}
void pushBack(Node **head, int x) {
    Node *newNode = makeNode(x);
    if (*head == nullptr) {
        *head = newNode;
        return;
    }
    Node *temp = *head;
    while (temp->next != nullptr)
        temp = temp->next;
    temp->next = newNode;
    newNode->prev = temp;
}
void insert(Node **head, int x, int k) {
    int n = nodeCount(*head);
    if (k < 1 || k > n + 1) return;
    if (k == 1) {
        pushFront(head, x);
        return;
    }
    if (k == n + 1) {
        pushBack(head, x);
        return;
    }
    Node *newNode = makeNode(x);
    Node *temp = *head;
    for (int i = 1; i < k; i++) {
        temp = temp->next;
    }
    newNode->next = temp;
    temp->prev = newNode;
    newNode->prev = temp->prev;
    temp->prev = newNode;
}
void popFront(Node **head) {
    if (*head == nullptr) return;
    Node *temp = *head;
    *head = (*head)->next;
    if (*head != nullptr)
        (*head)->prev = nullptr;
    delete temp;
}
void popBack(Node **head) {
    if (*head == nullptr) return;
    if ((*head)->next == nullptr) {
        popFront(head);
        return;
    }
    Node *temp = *head;
    while (temp->next != nullptr)
        temp = temp->next;
    temp->prev->next = nullptr;
    delete temp;
}
void erase(Node **head, int k) {
    int n = nodeCount(*head);
    if (k < 1 || k > n) return;
    if (k == 1) {
        popFront(head);
        return;
    }
    if (k == n) {
        popBack(head);
        return;
    }
    Node *temp = *head;
    for (int i = 1; i < k; i++) {
        temp = temp->next;
    }
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;
    delete temp;
}
void clear(Node **head) {
    while (*head != nullptr) {
        popFront(head);
    }
}
Node *search(Node *head, int x) {
    Node *temp = head;
    while (temp != nullptr) {
        if (temp->data == x) return temp;
        temp = temp->next;
    }
    return nullptr;
}


