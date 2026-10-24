/**
 * @file Random.h
 * @brief RequestFactory: small random helpers used to build requests.
 *
 * These are the self-contained helper functions the project rules allow to
 * be AI-assisted once the design is fixed. One @c std::mt19937 engine is
 * seeded once so a run is reproducible when a seed is given in the config.
 */
#ifndef AGF_RANDOM_H
#define AGF_RANDOM_H

#include <string>

#include "Request.h"

class Config;

/**
 * @brief Seed the shared random engine.
 * @param seed Seed value; 0 means draw a seed from @c std::random_device.
 * @return The seed actually used (useful for logging).
 */
unsigned seedRandom(unsigned seed);

/** @return A random dotted IPv4 address such as "172.16.4.9". */
std::string randomIPv4();

/**
 * @brief Uniform random task length.
 * @param minCycles Lower bound (inclusive).
 * @param maxCycles Upper bound (inclusive).
 * @return Cycles of work in [minCycles, maxCycles].
 */
int randomTaskTime(int minCycles, int maxCycles);

/** @return 'P' or 'S' with equal probability. */
char randomJobType();

/**
 * @brief Bernoulli trial.
 * @param probability Chance of success in [0, 1].
 * @return true with the given probability.
 */
bool randomChance(double probability);

/**
 * @brief How many requests arrive this cycle for a given arrival rate.
 *
 * A rate of 0.6 yields one request 60% of the time; a rate of 1.5 yields one
 * request always plus a second one 50% of the time, so rates above 1 model
 * more than one arrival per cycle.
 * @param rate Expected arrivals per cycle (may exceed 1).
 * @return Number of arrivals for this cycle.
 */
int randomArrivals(double rate);

/**
 * @brief Build a complete random Request from the configured task-time range.
 * @param cfg   Configuration (reads task_time_min / task_time_max).
 * @param cycle Current clock value, stored as Request::arrivedCycle.
 * @return A new request.
 */
Request makeRequest(const Config& cfg, int cycle);

#endif // AGF_RANDOM_H
