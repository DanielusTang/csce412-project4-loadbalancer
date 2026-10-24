/**
 * @file main.cpp
 * @brief AggieForge Traffic Cloud - driver for the load balancer simulation.
 *
 * Usage:
 * @code
 *   ./loadbalancer [config.txt] [--verbose|--quiet] [--servers N] [--cycles N]
 *                  [--set key=value]... [--no-prompt]
 * @endcode
 * The program loads the configuration file, asks for the number of web
 * servers and the run time in clock cycles (Enter keeps the configured
 * value), builds the Logger, IPBlocker and LoadBalancer, seeds the starting
 * queue with servers * initial_multiplier requests, runs the clock and
 * prints the START and END summaries to the console and the log file.
 * With @c switch_enabled = true (bonus) it builds a Switch with one
 * LoadBalancer for processing jobs and one for streaming jobs instead.
 */
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Config.h"
#include "IPBlocker.h"
#include "LoadBalancer.h"
#include "Logger.h"
#include "Random.h"
#include "Switch.h"

namespace {

void usage(const char* prog) {
    std::cerr << "usage: " << prog
              << " [config.txt] [--verbose|--quiet] [--servers N] [--cycles N]"
                 " [--set key=value]... [--no-prompt]\n";
}

/// Ask for a positive integer; an empty line keeps @p current.
int prompt(const std::string& label, int current) {
    while (true) {
        std::cout << label << " [" << current << "]: " << std::flush;
        std::string line;
        if (!std::getline(std::cin, line) || line.empty()) {
            std::cout << current << "\n";
            return current;
        }
        try {
            int v = std::stoi(line);
            if (v > 0) return v;
        } catch (...) {
        }
        std::cout << "  please enter a positive whole number\n";
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string cfgPath = "config.txt";
    bool noPrompt = false;
    bool haveServers = false, haveCycles = false;
    std::vector<std::pair<std::string, std::string>> overrides;   // flags beat the file

    // --- command line -----------------------------------------------------
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](const char* what) -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "error: " << what << " needs a value\n";
                usage(argv[0]);
                std::exit(2);
            }
            return argv[++i];
        };
        if (a == "--verbose")        overrides.emplace_back("mode", "verbose");
        else if (a == "--quiet")     overrides.emplace_back("mode", "quiet");
        else if (a == "--no-prompt") noPrompt = true;
        else if (a == "--servers")   { overrides.emplace_back("servers", next("--servers")); haveServers = true; }
        else if (a == "--cycles")    { overrides.emplace_back("cycles", next("--cycles")); haveCycles = true; }
        else if (a == "--set") {
            std::string kv = next("--set");
            std::size_t eq = kv.find('=');
            if (eq == std::string::npos) { std::cerr << "error: --set expects key=value\n"; return 2; }
            overrides.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
        }
        else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
        else if (!a.empty() && a[0] == '-') { std::cerr << "error: unknown option " << a << "\n"; usage(argv[0]); return 2; }
        else cfgPath = a;
    }

    // --- configuration (requirement 13) ------------------------------------
    Config cfg;
    if (!cfg.load(cfgPath)) {
        std::cerr << "warning: could not open " << cfgPath << ", using built-in defaults\n";
    }
    for (const auto& kv : overrides) cfg.set(kv.first, kv.second);

    // --- user input (requirement 4) ---------------------------------------
    std::cout << "AggieForge Traffic Cloud - load balancer simulation\n";
    int servers = cfg.getInt("servers", 20);
    int cycles  = cfg.getInt("cycles", 100000);
    if (!noPrompt) {
        if (!haveServers) servers = prompt("Number of web servers", servers);
        if (!haveCycles)  cycles  = prompt("Run time in clock cycles", cycles);
    }
    cfg.set("servers", std::to_string(servers));
    cfg.set("cycles", std::to_string(cycles));

    // --- wiring ------------------------------------------------------------
    unsigned seed = seedRandom(static_cast<unsigned>(cfg.getInt("seed", 412)));
    cfg.set("seed", std::to_string(seed));

    Level level = cfg.getString("mode", "quiet") == "verbose" ? Level::Verbose : Level::Quiet;
    Logger log(cfg.getString("log_file", "lb_log.txt"), level, cfg.getBool("color", true));

    IPBlocker blocker;
    for (const std::string& r : cfg.getList("blocked_ranges")) {
        if (!blocker.addRange(r)) std::cerr << "warning: ignoring bad blocked range '" << r << "'\n";
    }

    const int multiplier = cfg.getInt("initial_multiplier", 100);

    if (cfg.getBool("switch_enabled", false)) {
        // --- bonus: Switch + one balancer per job type -----------------------
        const int serversP = (servers + 1) / 2;
        const int serversS = servers - serversP > 0 ? servers - serversP : 1;
        LoadBalancer lbP(cfg, blocker, log, "AGF-LB-P", serversP);
        LoadBalancer lbS(cfg, blocker, log, "AGF-LB-S", serversS);
        lbP.setExternalArrivals(true);
        lbS.setExternalArrivals(true);
        lbP.seedQueue(serversP * multiplier, 'P');
        lbS.seedQueue(serversS * multiplier, 'S');

        Switch sw;
        sw.attach('P', lbP);
        sw.attach('S', lbS);

        log.summary("");
        log.summary("=== AggieForge Traffic Cloud - SWITCH enabled: 'P' -> AGF-LB-P (" +
                    std::to_string(serversP) + " servers), 'S' -> AGF-LB-S (" + std::to_string(serversS) +
                    " servers) ===");
        sw.summaryAll();                                       // START blocks
        const double rate = cfg.getDouble("arrival_probability", 0.6);
        for (int cycle = 0; cycle < cycles; ++cycle) {
            for (int n = randomArrivals(rate); n > 0; --n) {
                sw.route(makeRequest(cfg, cycle));             // firewall check inside addRequest
            }
            sw.stepAll();
        }
        sw.summaryAll();                                       // END blocks
        log.summary("");
        log.summary("=== SWITCH routing totals: P -> AGF-LB-P " + std::to_string(sw.routed('P')) +
                    ", S -> AGF-LB-S " + std::to_string(sw.routed('S')) + " ===");
    } else {
        // --- single load balancer ---------------------------------------------
        LoadBalancer lb(cfg, blocker, log, "AGF-LB");
        lb.seedQueue(servers * multiplier);

        lb.printSummary();          // START block
        lb.run(cycles);
        lb.printSummary();          // END block
    }

    if (!log.path().empty()) {
        std::cout << "\nLog written to " << log.path() << "\n";
    }
    return 0;
}
