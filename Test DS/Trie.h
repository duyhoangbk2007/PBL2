#include <iostream>
#include <string>
using namespace std;

struct Trie {
    struct Node
    {
        Node *child[26];
        bool isEnd;

        Node() {
            for (int i = 0; i < 26; i++)
            {
                child[i] = nullptr; 
            }
            isEnd = false;  
        }
    };
};
