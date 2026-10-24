/**
 * @file LoadBalancer.h
 * @brief The load balancer: owns a request queue and a pool of web servers.
 */
#ifndef AGF_LOAD_BALANCER_H
#define AGF_LOAD_BALANCER_H

#include <climits>
#include <cstddef>
#include <string>
#include <vector>

#include "Config.h"
#include "IPBlocker.h"
#include "Logger.h"
#include "Request.h"
#include "RequestQueue.h"
#include "WebServer.h"

/**
 * @struct Stats
 * @brief Counters collected by a LoadBalancer for the END summary.
 */
struct Stats {
    long long   generated   = 0;        ///< Requests created (seed + arrivals, incl. rejected).
    long long   generatedP  = 0;        ///< ... of which processing jobs.
    long long   generatedS  = 0;        ///< ... of which streaming jobs.
    long long   processed   = 0;        ///< Requests completed by a server.
    long long   rejected    = 0;        ///< Requests discarded by the firewall.
    int         scaleUps    = 0;        ///< Servers added by autoscaling.
    int         scaleDowns  = 0;        ///< Servers removed by autoscaling.
    int         minServers  = INT_MAX;  ///< Smallest active pool seen.
    int         maxServers  = 0;        ///< Largest active pool seen.
    std::size_t minQueue    = SIZE_MAX; ///< Shallowest queue seen.
    std::size_t maxQueue    = 0;        ///< Deepest queue seen.
    long long   totalWait   = 0;        ///< Sum of (assign cycle - arrival cycle).
    long long   assigned    = 0;        ///< Requests handed to a server.
    long long   busyCycles  = 0;        ///< Server-cycles spent working (incl. removed servers).
    long long   serverCycles = 0;       ///< Server-cycles available (sum of pool size per cycle).
};

/**
 * @class LoadBalancer
 * @brief Distributes queued requests over a dynamically sized pool of WebServer objects.
 *
 * The balancer owns its RequestQueue and its servers (composition) but only
 * references the Config, IPBlocker and Logger, which are injected through the
 * constructor. That split lets several LoadBalancer instances share one
 * firewall, one log file and one configuration while keeping private queues,
 * pools and statistics (requirement 8 and the bonus Switch).
 *
 * One call to stepCycle() performs: arrival -> firewall -> servers tick ->
 * dispatch -> autoscale -> STATUS heartbeat -> clock++.
 */
class LoadBalancer {
public:
    /**
     * @brief Create a balancer with @c cfg.servers idle web servers.
     * @param cfg     Shared configuration (servers, task-time range, thresholds, ...).
     * @param blocker Shared IP-range firewall.
     * @param log     Shared logger.
     * @param name    Display name used in log lines (e.g. "AGF-LB-P").
     */
    LoadBalancer(const Config& cfg, IPBlocker& blocker, Logger& log,
                 std::string name = "AGF-LB");

    /**
     * @brief Fill the starting queue with random requests (arrivedCycle = 0).
     * @param count Number of requests, normally servers * 100.
     */
    void seedQueue(int count);

    /**
     * @brief Admit a request through the firewall.
     * @param r The request.
     * @return false (and rejected++) if the source IP is blocked; true once queued.
     */
    bool addRequest(const Request& r);

    /** @brief Advance the simulation by one clock cycle. */
    void stepCycle();

    /**
     * @brief Run stepCycle() repeatedly.
     * @param cycles Number of cycles.
     */
    void run(int cycles);

    /**
     * @brief Switch off internal random arrivals.
     *
     * Used by the bonus Switch, which generates arrivals itself and routes
     * them by job type so each pool sees the right mix.
     * @param external true when another component feeds addRequest().
     */
    void setExternalArrivals(bool external);

    /** @return Requests waiting in the queue. */
    std::size_t queueSize() const;

    /** @return Servers currently in the pool. */
    int activeServers() const;

    /** @return Servers in the pool that are not processing anything. */
    int idleServers() const;

    /** @return Servers that were created and later removed by scale-down. */
    int inactiveServers() const;

    /** @return Current clock value. */
    int clock() const;

    /** @return Read-only statistics. */
    const Stats& stats() const;

    /** @return Display name. */
    const std::string& name() const;

    /**
     * @brief Write the START block (when the clock is 0) or the END block.
     *
     * The START block records the configuration so the log is self-describing;
     * the END block is the deliverable summary: ending queue, generated /
     * processed / rejected, active and removed servers, scale events, min/max
     * servers and queue, average wait and utilization.
     */
    void printSummary();

private:
    const Config&          cfg_;
    IPBlocker&             blocker_;
    Logger&                log_;
    std::string            name_;

    RequestQueue           queue_;
    std::vector<WebServer> servers_;
    Stats                  stats_;

    int    clock_          = 0;
    int    cooldownUntil_  = 0;
    int    nextId_         = 1;
    int    removed_        = 0;
    bool   externalArrivals_ = false;

    // cached configuration
    int    taskMin_, taskMax_, cooldownN_, scaleLow_, scaleHigh_, minServers_, maxServers_,
           statusEvery_, initialServers_;
    double arrivalRate_;

    void maybeGenerateRequest();
    void tickServers();
    void dispatch();
    void autoscale();
    void addServer();
    bool removeServer();
    void heartbeat();
    void sample();

    std::string row(const std::string& label, const std::string& value) const;
};

#endif // AGF_LOAD_BALANCER_H
