/**
 * @file Switch.cpp
 * @brief Implementation of Switch.
 */
#include "Switch.h"

#include <algorithm>

void Switch::attach(char jobType, LoadBalancer& lb) {
    routes_[jobType] = &lb;
    if (std::find(all_.begin(), all_.end(), &lb) == all_.end()) {
        all_.push_back(&lb);
    }
}

bool Switch::route(const Request& r) {
    if (all_.empty()) return false;
    auto it = routes_.find(r.jobType);
    LoadBalancer* target = (it != routes_.end()) ? it->second : all_.front();
    ++routed_[r.jobType];
    return target->addRequest(r);
}

void Switch::stepAll() {
    for (LoadBalancer* lb : all_) lb->stepCycle();
}

void Switch::summaryAll() {
    for (LoadBalancer* lb : all_) lb->printSummary();
}

long long Switch::routed(char jobType) const {
    auto it = routed_.find(jobType);
    return it == routed_.end() ? 0 : it->second;
}

const std::vector<LoadBalancer*>& Switch::balancers() const { return all_; }
