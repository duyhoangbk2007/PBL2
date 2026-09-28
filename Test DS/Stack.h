#include <iostream>

struct node {
    int data;
    node *next;
};

struct Stack {
    node *top;
    int count;
    int capacity;
};

node *makeNode(int x);
Stack *create(int capacity);
int isEmpty(Stack *s);
int isFull(Stack *s);
void push(Stack *s, int x);
void pop(Stack *s);
node *Top(Stack *s);
int size(Stack *s);
void clear(Stack *s);
void destroy(Stack *s);


node *makeNode(int x) {
    node *newNode = new node;
    newNode->data = x;
    newNode->next = nullptr;
    return newNode;
}

Stack *create(int capacity) {
    if (capacity < 0) capacity = 0;
    Stack *s = new Stack;
    s->capacity = capacity;
    s->count = 0;
    s->top = nullptr;
    return s;
}

int isFull(Stack *s) {
    return s->count >= s->capacity;
}

int isEmpty(Stack *s) {
    return s->count == 0;
}

void push(Stack *s, int x) {
    if (isFull(s)) return;

    node *newNode = makeNode(x);
    newNode->next = s->top;
    s->top = newNode;
    s->count++;
}

void pop(Stack *s) {
    if (isEmpty(s)) return;

    node *tmp = s->top;
    s->top = tmp->next;
    delete tmp;
    s->count--;
}

node *Top(Stack *s) {
    return s->top;
}

int size(Stack *s) {
    return s->count;
}

void clear(Stack *s) {
    while (!isEmpty(s)) {
        pop(s);
    }
}
void destroy(Stack *s) {
    if (s == nullptr) return;
    clear(s);
    delete s;
}

