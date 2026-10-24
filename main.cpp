/**
 * @file main.cpp
 * @brief AggieForge Traffic Cloud driver (milestone 1: fixed-size server pool).
 */
#include <iostream>
#include <vector>

#include "Config.h"
#include "Random.h"
#include "RequestQueue.h"
#include "WebServer.h"

int main(int argc, char* argv[]) {
    Config cfg;
    const std::string cfgPath = argc > 1 ? argv[1] : "config.txt";
    if (!cfg.load(cfgPath)) {
        std::cerr << "warning: could not open " << cfgPath << ", using built-in defaults\n";
    }
    seedRandom(static_cast<unsigned>(cfg.getInt("seed", 412)));

    int servers = cfg.getInt("servers", 20);
    int cycles  = cfg.getInt("cycles", 100000);
    std::cout << "Number of web servers [" << servers << "]: ";
    std::string line;
    if (std::getline(std::cin, line) && !line.empty()) servers = std::stoi(line);
    std::cout << "Run time in clock cycles [" << cycles << "]: ";
    if (std::getline(std::cin, line) && !line.empty()) cycles = std::stoi(line);

    RequestQueue queue;
    for (int i = 0; i < servers * cfg.getInt("initial_multiplier", 100); ++i) {
        queue.push(makeRequest(cfg, 0));
    }
    std::vector<WebServer> pool;
    for (int i = 1; i <= servers; ++i) pool.emplace_back(i);

    std::cout << "Starting queue size : " << queue.size() << "\n";
    long long processed = 0, generated = queue.size();
    double arrival = cfg.getDouble("arrival_probability", 0.6);
    for (int cycle = 0; cycle < cycles; ++cycle) {
        for (int n = randomArrivals(arrival); n > 0; --n) {
            queue.push(makeRequest(cfg, cycle));
            ++generated;
        }
        for (WebServer& s : pool) {
            if (s.tick(cycle)) ++processed;
        }
        for (WebServer& s : pool) {
            if (!s.isBusy() && !queue.empty()) s.assign(queue.pop(), cycle);
        }
    }
    std::cout << "Ending queue size   : " << queue.size() << "\n"
              << "Requests generated  : " << generated << "\n"
              << "Requests processed  : " << processed << "\n"
              << "Peak queue size     : " << queue.peakSize() << "\n";
    return 0;
}
