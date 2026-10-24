/**
 * @file Switch.h
 * @brief Bonus: a higher-level switch that routes requests by job type to separate load balancers.
 */
#ifndef AGF_SWITCH_H
#define AGF_SWITCH_H

#include <map>
#include <vector>

#include "LoadBalancer.h"
#include "Request.h"

/**
 * @class Switch
 * @brief Sorts incoming requests by Request::jobType and forwards each to the
 *        LoadBalancer registered for that type.
 *
 * Streaming ('S') jobs go to one balancer and its server pool, processing
 * ('P') jobs to another. All attached balancers are stepped in lock-step on
 * the same clock and each prints its own END summary so the statistics stay
 * separate. The Switch does not own the balancers; it only references them.
 */
class Switch {
public:
    /**
     * @brief Register the balancer that handles one job type.
     * @param jobType 'P' or 'S'.
     * @param lb      The balancer (must outlive the Switch).
     */
    void attach(char jobType, LoadBalancer& lb);

    /**
     * @brief Forward a request to the balancer for its job type.
     *
     * If no balancer is registered for the type, the first attached balancer
     * is used so no request is lost.
     * @param r The request.
     * @return The balancer's addRequest() result (false if the firewall rejected it).
     */
    bool route(const Request& r);

    /** @brief Advance every attached balancer by one clock cycle. */
    void stepAll();

    /** @brief Print the START (clock 0) or END summary of every attached balancer. */
    void summaryAll();

    /** @return Requests routed per job type, for the switch's own summary line. */
    long long routed(char jobType) const;

    /** @return All attached balancers in attach order. */
    const std::vector<LoadBalancer*>& balancers() const;

private:
    std::map<char, LoadBalancer*> routes_;
    std::vector<LoadBalancer*>    all_;
    std::map<char, long long>     routed_;
};

#endif // AGF_SWITCH_H
