/**
 * @file RequestQueue.cpp
 * @brief Implementation of RequestQueue.
 */
#include "RequestQueue.h"

void RequestQueue::push(const Request& r) {
    q_.push(r);
    if (q_.size() > peak_) {
        peak_ = q_.size();
    }
}

Request RequestQueue::pop() {
    Request r = q_.front();
    q_.pop();
    return r;
}

const Request& RequestQueue::front() const {
    return q_.front();
}

bool RequestQueue::empty() const {
    return q_.empty();
}

std::size_t RequestQueue::size() const {
    return q_.size();
}

std::size_t RequestQueue::peakSize() const {
    return peak_;
}
