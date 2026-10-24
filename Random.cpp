/**
 * @file Random.cpp
 * @brief Implementation of the RequestFactory helpers.
 */
#include "Random.h"

#include <random>
#include <sstream>

#include "Config.h"

namespace {
std::mt19937& engine() {
    static std::mt19937 eng(412u);
    return eng;
}
}  // namespace

unsigned seedRandom(unsigned seed) {
    if (seed == 0) {
        std::random_device rd;
        seed = rd();
        if (seed == 0) {
            seed = 1;
        }
    }
    engine().seed(seed);
    return seed;
}

std::string randomIPv4() {
    std::uniform_int_distribution<int> octet(0, 255);
    std::uniform_int_distribution<int> first(1, 254);
    std::ostringstream out;
    out << first(engine()) << '.' << octet(engine()) << '.' << octet(engine()) << '.'
        << octet(engine());
    return out.str();
}

int randomTaskTime(int minCycles, int maxCycles) {
    if (minCycles < 1) minCycles = 1;
    if (maxCycles < minCycles) maxCycles = minCycles;
    std::uniform_int_distribution<int> dist(minCycles, maxCycles);
    return dist(engine());
}

char randomJobType() {
    std::uniform_int_distribution<int> coin(0, 1);
    return coin(engine()) == 0 ? 'P' : 'S';
}

bool randomChance(double probability) {
    if (probability <= 0.0) return false;
    if (probability >= 1.0) return true;
    std::bernoulli_distribution d(probability);
    return d(engine());
}

int randomArrivals(double rate) {
    if (rate <= 0.0) return 0;
    int whole = static_cast<int>(rate);
    double frac = rate - whole;
    return whole + (randomChance(frac) ? 1 : 0);
}

Request makeRequest(const Config& cfg, int cycle) {
    Request r;
    r.ipIn         = randomIPv4();
    r.ipOut        = randomIPv4();
    r.time         = randomTaskTime(cfg.getInt("task_time_min", 3), cfg.getInt("task_time_max", 80));
    r.jobType      = randomJobType();
    r.arrivedCycle = cycle;
    return r;
}
