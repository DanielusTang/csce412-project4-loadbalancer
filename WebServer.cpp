/**
 * @file WebServer.cpp
 * @brief Implementation of WebServer.
 */
#include "WebServer.h"

WebServer::WebServer(int id) : id_(id) {}

void WebServer::assign(const Request& r, int cycle) {
    current_     = r;
    remaining_   = (r.time > 0) ? r.time : 1;   // every request costs at least one cycle
    finishCycle_ = cycle + remaining_;
    busy_        = true;
}

bool WebServer::tick(int /*cycle*/) {
    if (!busy_) {
        return false;
    }
    --remaining_;
    ++busyCycles_;
    if (remaining_ <= 0) {
        busy_ = false;
        ++handled_;
        return true;
    }
    return false;
}

bool WebServer::isBusy() const { return busy_; }
int WebServer::id() const { return id_; }
int WebServer::requestsHandled() const { return handled_; }
long long WebServer::busyCycles() const { return busyCycles_; }
const Request& WebServer::current() const { return current_; }
int WebServer::finishCycle() const { return finishCycle_; }
