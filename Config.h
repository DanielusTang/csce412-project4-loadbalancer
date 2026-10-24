/**
 * @file Config.h
 * @brief Key/value configuration file reader (requirement 13: configurable without editing code).
 */
#ifndef AGF_CONFIG_H
#define AGF_CONFIG_H

#include <map>
#include <string>
#include <vector>

/**
 * @class Config
 * @brief Parses a @c key = value text file and serves typed lookups with defaults.
 *
 * Blank lines and anything after a @c # are ignored, so the shipped
 * @c config.txt can be commented. Every getter takes a fallback, so a partial
 * file still runs. set() lets the driver override values with the user's
 * interactive input (number of servers, run length) or command-line flags.
 */
class Config {
public:
    /**
     * @brief Read a configuration file.
     * @param path Path to the file (e.g. "config.txt").
     * @return true if the file could be opened.
     */
    bool load(const std::string& path);

    /** @return true if @p key was present in the file or set with set(). */
    bool has(const std::string& key) const;

    /**
     * @brief Override or add a value.
     * @param key   Setting name.
     * @param value Raw string value.
     */
    void set(const std::string& key, const std::string& value);

    /** @return Integer value of @p key, or @p def when missing/unparsable. */
    int getInt(const std::string& key, int def) const;

    /** @return Floating-point value of @p key, or @p def when missing/unparsable. */
    double getDouble(const std::string& key, double def) const;

    /** @return Boolean value (true/yes/on/1) of @p key, or @p def when missing. */
    bool getBool(const std::string& key, bool def) const;

    /** @return String value of @p key, or @p def when missing. */
    std::string getString(const std::string& key, const std::string& def) const;

    /**
     * @brief Split a comma-separated value into trimmed items.
     * @param key Setting name (e.g. "blocked_ranges").
     * @return The items; empty when the key is missing.
     */
    std::vector<std::string> getList(const std::string& key) const;

    /** @return Path given to the last successful load(). */
    const std::string& path() const;

private:
    std::map<std::string, std::string> values_;
    std::string path_;

    static std::string trim(const std::string& s);
};

#endif // AGF_CONFIG_H
