#ifndef TRIE_H
#define TRIE_H

#include <string>

// ==================== KHAI BAO ====================

// Moi nhanh tuong ung mot byte, bao gom cac byte UTF-8.
const int TRIE_ALPHABET_SIZE = 256;

struct TrieNode {
    TrieNode *child[TRIE_ALPHABET_SIZE];
    bool isEndOfWord;
    std::string originalName;
};

TrieNode *makeTrieNode();
int trieIndex(unsigned char c);
void addString(TrieNode **root, const std::string &name);
bool searchString(TrieNode *root, const std::string &name);
bool removeString(TrieNode *root, const std::string &name);
void collectWords(TrieNode *root, std::string results[], int limit, int &count);
int autoComplete(TrieNode *root, const std::string &prefix, std::string results[], int limit);
void clearTrie(TrieNode **root);

// ==================== DINH NGHIA ====================

TrieNode *makeTrieNode() {
    TrieNode *newNode = new TrieNode;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        newNode->child[i] = nullptr;
    }
    newNode->isEndOfWord = false;
    newNode->originalName = "";
    return newNode;
}

int trieIndex(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

void addString(TrieNode **root, const std::string &name) {
    if (name.empty()) return;
    if (*root == nullptr) *root = makeTrieNode();
    TrieNode *temp = *root;
    for (unsigned char c : name) {
        int index = trieIndex(c);
        if (temp->child[index] == nullptr) {
            temp->child[index] = makeTrieNode();
        }
        temp = temp->child[index];
    }
    temp->isEndOfWord = true;
    temp->originalName = name;
}

bool searchString(TrieNode *root, const std::string &name) {
    if (root == nullptr || name.empty()) return false;
    TrieNode *temp = root;
    for (unsigned char c : name) {
        int index = trieIndex(c);
        if (temp->child[index] == nullptr) return false;
        temp = temp->child[index];
    }
    return temp->isEndOfWord;
}

bool removeString(TrieNode *root, const std::string &name) {
    if (root == nullptr || name.empty()) return false;
    TrieNode *temp = root;
    for (unsigned char c : name) {
        int index = trieIndex(c);
        if (temp->child[index] == nullptr) return false;
        temp = temp->child[index];
    }
    if (!temp->isEndOfWord) return false;
    temp->isEndOfWord = false;
    temp->originalName = "";
    return true;
}

void collectWords(TrieNode *root, std::string results[], int limit, int &count) {
    if (root == nullptr || results == nullptr || count >= limit) return;
    if (root->isEndOfWord) {
        results[count] = root->originalName;
        count++;
    }
    for (int i = 0; i < TRIE_ALPHABET_SIZE && count < limit; i++) {
        if (root->child[i] != nullptr) {
            collectWords(root->child[i], results, limit, count);
        }
    }
}

int autoComplete(TrieNode *root, const std::string &prefix, std::string results[], int limit) {
    if (root == nullptr || results == nullptr || limit <= 0) return 0;
    TrieNode *temp = root;
    for (unsigned char c : prefix) {
        int index = trieIndex(c);
        if (temp->child[index] == nullptr) return 0;
        temp = temp->child[index];
    }
    int count = 0;
    collectWords(temp, results, limit, count);
    return count;
}

void clearTrie(TrieNode **root) {
    if (*root == nullptr) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        clearTrie(&((*root)->child[i]));
    }
    delete *root;
    *root = nullptr;
}

#endif
