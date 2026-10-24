/**
 * @file Logger.h
 * @brief Single output sink: coloured console text plus a plain-text log file.
 */
#ifndef AGF_LOGGER_H
#define AGF_LOGGER_H

#include <fstream>
#include <string>

/** @brief Logging verbosity. Quiet prints only the START/END summaries. */
enum class Level {
    Quiet,   ///< Summaries only (the deliverable log).
    Verbose  ///< Summaries plus per-event lines and the STATUS heartbeat.
};

/**
 * @class Logger
 * @brief Writes every line to the console (with ANSI colour by event type) and
 *        to the log file (without colour codes so the file stays readable).
 *
 * Event categories and their colours (requirement 15): ARRIVE/ASSIGN green,
 * DONE cyan, BLOCK red, SCALE+/SCALE- yellow, STATUS magenta. The level is a
 * constructor argument so the LoadBalancer contains no verbosity branches.
 */
class Logger {
public:
    /**
     * @brief Open the log file and write the branded header.
     * @param filePath Log file path; empty disables the file.
     * @param level    Quiet or Verbose.
     * @param color    Whether to emit ANSI colour codes on the console.
     */
    Logger(const std::string& filePath, Level level, bool color);

    /** @brief Flushes and closes the log file. */
    ~Logger();

    /**
     * @brief Log one event (verbose mode only).
     * @param category ARRIVE, ASSIGN, DONE, BLOCK, SCALE+, SCALE-, ...
     * @param msg      Event text.
     * @param cycle    Current clock value.
     */
    void event(const std::string& category, const std::string& msg, int cycle);

    /**
     * @brief Periodic STATUS heartbeat (verbose mode only).
     * @param msg   Status text.
     * @param cycle Current clock value.
     */
    void status(const std::string& msg, int cycle);

    /**
     * @brief Summary line, always written in both modes.
     * @param msg Text (START/END summary rows, headers).
     */
    void summary(const std::string& msg);

    /** @return true in verbose mode. */
    bool isVerbose() const;

    /** @return The log file path ("" when disabled). */
    const std::string& path() const;

private:
    std::ofstream file_;
    std::string   path_;
    Level         level_;
    bool          color_;

    void write(const std::string& plain, const std::string& colorCode);
    static std::string colorFor(const std::string& category);
};

#endif // AGF_LOGGER_H
