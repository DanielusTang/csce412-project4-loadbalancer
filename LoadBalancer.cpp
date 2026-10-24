/**
 * @file LoadBalancer.cpp
 * @brief Implementation of LoadBalancer.
 */
#include "LoadBalancer.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "Random.h"

namespace {
std::string srv(int id) {
    std::ostringstream s;
    s << "srv " << std::setw(2) << std::setfill('0') << id;
    return s.str();
}
std::string fixed(double v, int places) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(places) << v;
    return s.str();
}
}  // namespace

LoadBalancer::LoadBalancer(const Config& cfg, IPBlocker& blocker, Logger& log, std::string name)
    : cfg_(cfg), blocker_(blocker), log_(log), name_(std::move(name)) {
    initialServers_ = std::max(1, cfg_.getInt("servers", 20));
    taskMin_        = cfg_.getInt("task_time_min", 3);
    taskMax_        = cfg_.getInt("task_time_max", 80);
    arrivalRate_    = cfg_.getDouble("arrival_probability", 0.6);
    cooldownN_      = std::max(0, cfg_.getInt("cooldown_n", 50));
    scaleLow_       = cfg_.getInt("scale_low", 50);
    scaleHigh_      = cfg_.getInt("scale_high", 80);
    minServers_     = std::max(1, cfg_.getInt("min_servers", 1));
    maxServers_     = std::max(minServers_, cfg_.getInt("max_servers", 200));
    statusEvery_    = std::max(1, cfg_.getInt("status_every", 1000));

    for (int i = 0; i < initialServers_; ++i) {
        servers_.emplace_back(nextId_++);
    }
}

void LoadBalancer::seedQueue(int count) {
    for (int i = 0; i < count; ++i) {
        Request r = makeRequest(cfg_, 0);
        ++stats_.generated;
        (r.jobType == 'P' ? stats_.generatedP : stats_.generatedS)++;
        queue_.push(r);
    }
}

bool LoadBalancer::addRequest(const Request& r) {
    ++stats_.generated;
    (r.jobType == 'P' ? stats_.generatedP : stats_.generatedS)++;
    if (blocker_.isBlocked(r.ipIn)) {
        ++stats_.rejected;
        log_.event("BLOCK", name_ + ": " + r.ipIn + " -> " + r.ipOut + " rejected by firewall (total " +
                            std::to_string(stats_.rejected) + ")",
                   clock_);
        return false;
    }
    queue_.push(r);
    log_.event("ARRIVE", name_ + ": " + r.ipIn + " -> " + r.ipOut + " (" + r.jobType + ", " +
                             std::to_string(r.time) + " cyc) queued, depth " +
                             std::to_string(queue_.size()),
               clock_);
    return true;
}

void LoadBalancer::setExternalArrivals(bool external) { externalArrivals_ = external; }

void LoadBalancer::maybeGenerateRequest() {
    if (externalArrivals_) return;
    for (int n = randomArrivals(arrivalRate_); n > 0; --n) {
        addRequest(makeRequest(cfg_, clock_));
    }
}

void LoadBalancer::tickServers() {
    for (WebServer& s : servers_) {
        if (s.tick(clock_)) {
            ++stats_.processed;
            const Request& r = s.current();
            log_.event("DONE", name_ + ": " + srv(s.id()) + " finished " + r.ipIn + " (" + r.jobType +
                                   ", " + std::to_string(r.time) + " cyc); asking for next",
                       clock_);
        }
    }
}

void LoadBalancer::dispatch() {
    for (WebServer& s : servers_) {
        if (queue_.empty()) break;
        if (s.isBusy()) continue;
        Request r = queue_.pop();
        int waited = clock_ - r.arrivedCycle;
        stats_.totalWait += waited;
        ++stats_.assigned;
        s.assign(r, clock_);
        log_.event("ASSIGN", name_ + ": " + srv(s.id()) + " <- " + r.ipIn + " -> " + r.ipOut + " (" +
                                 r.jobType + ", " + std::to_string(r.time) + " cyc, waited " +
                                 std::to_string(waited) + ")",
                   clock_);
    }
}

void LoadBalancer::autoscale() {
    if (clock_ < cooldownUntil_) return;
    const int n = static_cast<int>(servers_.size());
    const long long q = static_cast<long long>(queue_.size());
    const long long low  = static_cast<long long>(scaleLow_) * n;
    const long long high = static_cast<long long>(scaleHigh_) * n;

    if (q < low && n > minServers_) {
        if (removeServer()) {
            cooldownUntil_ = clock_ + cooldownN_;
        }
        // all servers busy: removal is deferred to the next check
    } else if (q > high && n < maxServers_) {
        addServer();
        cooldownUntil_ = clock_ + cooldownN_;
    }
}

void LoadBalancer::addServer() {
    const int n = static_cast<int>(servers_.size());
    servers_.emplace_back(nextId_++);
    ++stats_.scaleUps;
    log_.event("SCALE+", name_ + ": queue " + std::to_string(queue_.size()) + " > " +
                             std::to_string(scaleHigh_) + "*" + std::to_string(n) + "=" +
                             std::to_string(static_cast<long long>(scaleHigh_) * n) + " -> add " +
                             srv(servers_.back().id()) + " (now " + std::to_string(n + 1) +
                             " servers, cooldown " + std::to_string(cooldownN_) + ")",
               clock_);
}

bool LoadBalancer::removeServer() {
    auto it = std::find_if(servers_.begin(), servers_.end(),
                           [](const WebServer& s) { return !s.isBusy(); });
    if (it == servers_.end()) {
        return false;   // never drop a busy server's request
    }
    const int n = static_cast<int>(servers_.size());
    const int id = it->id();
    stats_.busyCycles += it->busyCycles();
    servers_.erase(it);
    ++removed_;
    ++stats_.scaleDowns;
    log_.event("SCALE-", name_ + ": queue " + std::to_string(queue_.size()) + " < " +
                             std::to_string(scaleLow_) + "*" + std::to_string(n) + "=" +
                             std::to_string(static_cast<long long>(scaleLow_) * n) +
                             " -> remove idle " + srv(id) + " (now " + std::to_string(n - 1) +
                             " servers, cooldown " + std::to_string(cooldownN_) + ")",
               clock_);
    return true;
}

void LoadBalancer::heartbeat() {
    if (!log_.isVerbose() || clock_ == 0 || clock_ % statusEvery_ != 0) return;
    std::ostringstream s;
    s << name_ << ": queue=" << queue_.size() << " active=" << servers_.size()
      << " idle=" << idleServers() << " removed=" << removed_ << " processed=" << stats_.processed
      << " rejected=" << stats_.rejected;
    log_.status(s.str(), clock_);
}

void LoadBalancer::sample() {
    const int n = static_cast<int>(servers_.size());
    stats_.minServers = std::min(stats_.minServers, n);
    stats_.maxServers = std::max(stats_.maxServers, n);
    stats_.minQueue   = std::min(stats_.minQueue, queue_.size());
    stats_.maxQueue   = std::max(stats_.maxQueue, queue_.size());
    stats_.serverCycles += n;
}

void LoadBalancer::stepCycle() {
    maybeGenerateRequest();
    tickServers();
    dispatch();
    sample();
    autoscale();
    heartbeat();
    ++clock_;
}

void LoadBalancer::run(int cycles) {
    for (int i = 0; i < cycles; ++i) {
        stepCycle();
    }
}

std::size_t LoadBalancer::queueSize() const { return queue_.size(); }
int LoadBalancer::activeServers() const { return static_cast<int>(servers_.size()); }
int LoadBalancer::idleServers() const {
    return static_cast<int>(std::count_if(servers_.begin(), servers_.end(),
                                          [](const WebServer& s) { return !s.isBusy(); }));
}
int LoadBalancer::inactiveServers() const { return removed_; }
int LoadBalancer::clock() const { return clock_; }
const Stats& LoadBalancer::stats() const { return stats_; }
const std::string& LoadBalancer::name() const { return name_; }

std::string LoadBalancer::row(const std::string& label, const std::string& value) const {
    std::ostringstream s;
    s << std::left << std::setw(22) << label << ": " << value;
    return s.str();
}

void LoadBalancer::printSummary() {
    if (clock_ == 0) {
        std::string ranges;
        for (const std::string& r : blocker_.ranges()) {
            ranges += (ranges.empty() ? "" : ", ") + r;
        }
        if (ranges.empty()) ranges = "(none)";
        log_.summary("");
        log_.summary("=== AggieForge Traffic Cloud - " + name_ + " START ===");
        log_.summary(row("Config file", cfg_.path().empty() ? "(defaults)" : cfg_.path()));
        log_.summary(row("Servers (initial)", std::to_string(initialServers_)));
        log_.summary(row("Run length", cfg_.getString("cycles", "?") + " cycles"));
        log_.summary(row("Starting queue size", std::to_string(queue_.size())));
        log_.summary(row("Task time range", std::to_string(taskMin_) + " - " + std::to_string(taskMax_) + " cycles"));
        log_.summary(row("Arrival probability", fixed(arrivalRate_, 2) + " / cycle"));
        log_.summary(row("Scale thresholds", std::to_string(scaleLow_) + " - " + std::to_string(scaleHigh_) +
                                              " per server, cooldown n = " + std::to_string(cooldownN_)));
        log_.summary(row("Server bounds", std::to_string(minServers_) + " - " + std::to_string(maxServers_)));
        log_.summary(row("Blocked IP ranges", ranges));
        log_.summary(row("RNG seed", cfg_.getString("seed", "412")));
        return;
    }

    const int busy = activeServers() - idleServers();
    const double avgWait = stats_.assigned ? static_cast<double>(stats_.totalWait) / stats_.assigned : 0.0;
    long long busyCycles = stats_.busyCycles;
    for (const WebServer& s : servers_) busyCycles += s.busyCycles();
    const double util = stats_.serverCycles ? 100.0 * busyCycles / stats_.serverCycles : 0.0;
    const long long inFlight = stats_.generated - stats_.rejected - stats_.processed -
                               static_cast<long long>(queue_.size()) - busy;

    log_.summary("");
    log_.summary("=== AggieForge Traffic Cloud - " + name_ + " END (after " + std::to_string(clock_) + " cycles) ===");
    log_.summary(row("Ending queue size", std::to_string(queue_.size())));
    log_.summary(row("Requests generated", std::to_string(stats_.generated) + "   (P: " +
                                               std::to_string(stats_.generatedP) + ", S: " +
                                               std::to_string(stats_.generatedS) + ")"));
    log_.summary(row("Requests processed", std::to_string(stats_.processed)));
    log_.summary(row("Requests rejected", std::to_string(stats_.rejected) + "   (firewall)"));
    log_.summary(row("Requests in service", std::to_string(busy) + "   (busy servers at end)"));
    log_.summary(row("Active servers (end)", std::to_string(activeServers()) + "   (busy " +
                                                 std::to_string(busy) + ", idle " +
                                                 std::to_string(idleServers()) + ")"));
    log_.summary(row("Inactive (removed)", std::to_string(removed_) + "   of " +
                                               std::to_string(nextId_ - 1) + " servers ever created"));
    log_.summary(row("Scale-ups / downs", std::to_string(stats_.scaleUps) + " / " + std::to_string(stats_.scaleDowns)));
    log_.summary(row("Min / max servers", std::to_string(stats_.minServers) + " / " + std::to_string(stats_.maxServers)));
    log_.summary(row("Min / max queue", std::to_string(stats_.minQueue) + " / " + std::to_string(stats_.maxQueue)));
    log_.summary(row("Avg wait in queue", fixed(avgWait, 1) + " cycles"));
    log_.summary(row("Server utilization", fixed(util, 1) + " %"));
    if (inFlight != 0) {
        log_.summary(row("Accounting error", std::to_string(inFlight)));
    }
}
