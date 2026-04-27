#ifndef STORAGE_STACK_H
#define STORAGE_STACK_H

// inverted singly linked list

struct ISLLNode {
    int data;
    ISLLNode* next;
};

class ISLL {
private:
    ISLLNode* bottom_;
    ISLLNode* top_;
    int size_;
public:
    ISLL();
    void push(int value);
    ISLLNode* begin() const;
    ISLLNode* end() const;
    bool empty() const;
    int size() const; 
};

#endif