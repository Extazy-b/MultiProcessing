#include "ISLL.h"

ISLL::ISLL() {
    bottom_ = new ISLLNode{0, nullptr};
    top_ = bottom_;
}

void ISLL::push(int value) {
    ISLLNode* node = new ISLLNode{value, top_};
    top_ = node;
}

ISLLNode* ISLL::begin() const { return top_; }
ISLLNode* ISLL::end() const { return bottom_; }
bool ISLL::empty() const { return top_ == bottom_; }