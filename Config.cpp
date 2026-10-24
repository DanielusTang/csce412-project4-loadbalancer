/**
 * @file Config.cpp
 * @brief Implementation of Config.
 */
#include "Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

std::string Config::trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) {
        return "";
    }
    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

bool Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    path_ = path;
    std::string line;
    while (std::getline(in, line)) {
        std::size_t hash = line.find('#');
        if (hash != std::string::npos) {
            line.erase(hash);
        }
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (!key.empty()) {
            values_[key] = val;
        }
    }
    return true;
}

bool Config::has(const std::string& key) const {
    return values_.count(key) != 0;
}

void Config::set(const std::string& key, const std::string& value) {
    values_[key] = trim(value);
}

int Config::getInt(const std::string& key, int def) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        return def;
    }
}

double Config::getDouble(const std::string& key, double def) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    try {
        return std::stod(it->second);
    } catch (...) {
        return def;
    }
}

bool Config::getBool(const std::string& key, bool def) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    std::string v = it->second;
    std::transform(v.begin(), v.end(), v.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (v == "true" || v == "yes" || v == "on" || v == "1") return true;
    if (v == "false" || v == "no" || v == "off" || v == "0") return false;
    return def;
}

std::string Config::getString(const std::string& key, const std::string& def) const {
    auto it = values_.find(key);
    return it == values_.end() ? def : it->second;
}

std::vector<std::string> Config::getList(const std::string& key) const {
    std::vector<std::string> out;
    auto it = values_.find(key);
    if (it == values_.end()) {
        return out;
    }
    std::stringstream ss(it->second);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            out.push_back(item);
        }
    }
    return out;
}

const std::string& Config::path() const {
    return path_;
}
