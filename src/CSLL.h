#ifndef READER_RING_H
#define READER_RING_H

#include "ISLL.h"

// cyclic singly linked list

struct CSLLNode {
    int rank;
    ISLLNode* current;
    CSLLNode* next;
};

class CSLL {
private:
    CSLLNode* current_;
public:
    CSLL();
    bool empty() const;
    void add(int rank, ISLLNode* startPos);
    CSLLNode* current() const;
    void advance();
    bool removeCurrent();
};

#endif