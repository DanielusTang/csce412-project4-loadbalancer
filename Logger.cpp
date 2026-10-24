/**
 * @file Logger.cpp
 * @brief Implementation of Logger.
 */
#include "Logger.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace {
const char* RESET   = "\033[0m";
const char* GREEN   = "\033[32m";
const char* CYAN    = "\033[36m";
const char* RED     = "\033[31m";
const char* YELLOW  = "\033[33m";
const char* MAGENTA = "\033[35m";
const char* BOLD    = "\033[1m";
}  // namespace

Logger::Logger(const std::string& filePath, Level level, bool color)
    : path_(filePath), level_(level), color_(color) {
#ifndef _WIN32
    if (color_ && !isatty(fileno(stdout))) {
        color_ = false;   // piped output: never emit escape codes
    }
#endif
    if (!path_.empty()) {
        file_.open(path_, std::ios::out | std::ios::trunc);
        if (!file_) {
            std::cerr << "warning: cannot open log file " << path_ << "\n";
            path_.clear();
        }
    }
    std::time_t now = std::time(nullptr);
    char stamp[32];
    std::strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    write("==========================================================", BOLD);
    write("  AggieForge Traffic Cloud - Load Balancer Simulation", BOLD);
    write("  Everything you need to stay balanced - every request, the right server.", "");
    write(std::string("  CSCE 412 Project 4 | run started ") + stamp + " | mode: " +
              (level_ == Level::Verbose ? "verbose" : "quiet"),
          "");
    write("==========================================================", BOLD);
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

std::string Logger::colorFor(const std::string& category) {
    if (category == "ARRIVE" || category == "ASSIGN") return GREEN;
    if (category == "DONE") return CYAN;
    if (category == "BLOCK") return RED;
    if (category == "SCALE+" || category == "SCALE-") return YELLOW;
    if (category == "STATUS") return MAGENTA;
    return "";
}

void Logger::write(const std::string& plain, const std::string& colorCode) {
    if (color_ && !colorCode.empty()) {
        std::cout << colorCode << plain << RESET << '\n';
    } else {
        std::cout << plain << '\n';
    }
    if (file_.is_open()) {
        file_ << plain << '\n';
    }
}

void Logger::event(const std::string& category, const std::string& msg, int cycle) {
    if (level_ != Level::Verbose) return;
    std::ostringstream line;
    line << "[cycle " << std::setw(7) << cycle << "] [" << std::left << std::setw(7)
         << category << std::right << "] " << msg;
    write(line.str(), colorFor(category));
}

void Logger::status(const std::string& msg, int cycle) {
    if (level_ != Level::Verbose) return;
    std::ostringstream line;
    line << "[cycle " << std::setw(7) << cycle << "] [STATUS ] " << msg;
    write(line.str(), colorFor("STATUS"));
}

void Logger::summary(const std::string& msg) {
    write(msg, "");
}

bool Logger::isVerbose() const { return level_ == Level::Verbose; }
const std::string& Logger::path() const { return path_; }
