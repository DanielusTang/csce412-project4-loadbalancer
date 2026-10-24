/**
 * @file IPBlocker.cpp
 * @brief Implementation of IPBlocker.
 */
#include "IPBlocker.h"

#include <sstream>

bool IPBlocker::parseIPv4(const std::string& ip, std::uint32_t& out) {
    std::stringstream ss(ip);
    std::string part;
    std::uint32_t value = 0;
    int count = 0;
    while (std::getline(ss, part, '.')) {
        if (part.empty() || part.size() > 3 || ++count > 4) return false;
        for (char c : part) {
            if (c < '0' || c > '9') return false;
        }
        int octet = std::stoi(part);
        if (octet < 0 || octet > 255) return false;
        value = (value << 8) | static_cast<std::uint32_t>(octet);
    }
    if (count != 4) return false;
    out = value;
    return true;
}

bool IPBlocker::addRange(const std::string& range) {
    std::string addr = range;
    int bits = 32;

    std::size_t slash = range.find('/');
    if (slash != std::string::npos) {
        addr = range.substr(0, slash);
        try {
            bits = std::stoi(range.substr(slash + 1));
        } catch (...) {
            return false;
        }
        if (bits < 0 || bits > 32) return false;
    } else if (!addr.empty() && addr.back() == '.') {
        // Dotted prefix such as "10.0." -> count the octets given, pad with zeros.
        int octets = 0;
        for (char c : addr) {
            if (c == '.') ++octets;
        }
        bits = octets * 8;
        while (octets++ < 4) addr += "0.";
        addr.pop_back();
    }

    std::uint32_t base = 0;
    if (!parseIPv4(addr, base)) return false;
    std::uint32_t mask = (bits == 0) ? 0u : (0xFFFFFFFFu << (32 - bits));
    ranges_.push_back({base & mask, mask});
    text_.push_back(range);
    return true;
}

bool IPBlocker::isBlocked(const std::string& ip) {
    std::uint32_t value = 0;
    if (!parseIPv4(ip, value)) {
        return false;
    }
    for (const Range& r : ranges_) {
        if ((value & r.mask) == r.base) {
            ++blocked_;
            return true;
        }
    }
    return false;
}

long long IPBlocker::blockedCount() const { return blocked_; }
const std::vector<std::string>& IPBlocker::ranges() const { return text_; }
