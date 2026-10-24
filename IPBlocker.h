/**
 * @file IPBlocker.h
 * @brief IP-range blocker: the firewall / DOS-prevention component (requirement 14).
 */
#ifndef AGF_IP_BLOCKER_H
#define AGF_IP_BLOCKER_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @class IPBlocker
 * @brief Rejects requests whose source address falls inside a blocked range.
 *
 * Ranges are stored as (base, mask) 32-bit integers so a check is one bitwise
 * AND per range. Blocking happens at admission time, before a request ever
 * touches the queue, so a flood from a blocked range never consumes server
 * capacity. One IPBlocker can be shared by several LoadBalancer instances.
 */
class IPBlocker {
public:
    /**
     * @brief Add a blocked range.
     * @param range CIDR notation ("10.0.0.0/8"), a bare address ("1.2.3.4"),
     *              or a dotted prefix ("192.168.").
     * @return true if the range could be parsed and was stored.
     */
    bool addRange(const std::string& range);

    /**
     * @brief Test an address against every stored range.
     * @param ip Dotted IPv4 string.
     * @return true if the address is blocked; also increments blockedCount().
     */
    bool isBlocked(const std::string& ip);

    /** @return Number of addresses that have been rejected so far. */
    long long blockedCount() const;

    /** @return The ranges as originally given, for the START summary. */
    const std::vector<std::string>& ranges() const;

    /**
     * @brief Convert a dotted IPv4 string to a 32-bit integer.
     * @param ip  Dotted string.
     * @param out Receives the value on success.
     * @return true if @p ip had four valid octets.
     */
    static bool parseIPv4(const std::string& ip, std::uint32_t& out);

private:
    struct Range {
        std::uint32_t base;
        std::uint32_t mask;
    };
    std::vector<Range>       ranges_;
    std::vector<std::string> text_;
    long long                blocked_ = 0;
};

#endif // AGF_IP_BLOCKER_H
