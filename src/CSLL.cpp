#include "CSLL.h"

CSLL::CSLL() : current_(nullptr) {}

bool CSLL::empty() const { return current_ == nullptr; }

void CSLL::add(int rank, ISLLNode* startPos) {
    CSLLNode* node = new CSLLNode{rank, startPos, nullptr};
    if (current_ == nullptr) {
        node->next = node;
        current_ = node;
    } else {
        node->next = current_->next;
        current_->next = node;
    }
}

CSLLNode* CSLL::current() const { return current_; }

void CSLL::advance() {
    if (current_ != nullptr)
        current_ = current_->next;
}

bool CSLL::removeCurrent() {
    if (current_ == nullptr) return true;

    CSLLNode* toDelete = current_;
    if (current_->next == current_) {
        delete toDelete;
        current_ = nullptr;
        return true;
    } else {
        CSLLNode* prev = current_;
        while (prev->next != current_)
            prev = prev->next;
        prev->next = current_->next;
        current_ = current_->next;
        delete toDelete;
        return false;
    }
}