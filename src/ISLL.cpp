#include "ISLL.h"

ISLL::ISLL() {
    bottom_ = new ISLLNode{0, nullptr};
    top_ = bottom_;
    size_ = 0;
}

void ISLL::push(int value) {
    ISLLNode* node = new ISLLNode{value, top_};
    top_ = node;
    size_++;
}

ISLLNode* ISLL::begin() const { return top_; }
ISLLNode* ISLL::end() const { return bottom_; }
bool ISLL::empty() const { return top_ == bottom_; }
int ISLL::size() const { return size_; }