/**
 * @file WebServer.h
 * @brief A web server that processes one Request at a time.
 */
#ifndef AGF_WEB_SERVER_H
#define AGF_WEB_SERVER_H

#include "Request.h"

/**
 * @class WebServer
 * @brief Processes exactly one request at a time.
 *
 * The server knows nothing about the queue or about other servers; the
 * LoadBalancer feeds it with assign() and advances it with tick(). When the
 * current request completes, tick() returns true exactly once and the server
 * becomes idle again, which is how it "asks for the next request".
 */
class WebServer {
public:
    /**
     * @brief Create an idle server.
     * @param id 1-based identifier, printed in logs as "srv 03".
     */
    explicit WebServer(int id);

    /**
     * @brief Hand a request to this server.
     * @param r     The request to process.
     * @param cycle The current clock value.
     * @pre !isBusy()
     */
    void assign(const Request& r, int cycle);

    /**
     * @brief Advance the server by one clock cycle.
     * @param cycle The current clock value (used for the finish timestamp).
     * @return true on the single cycle in which the current request finishes.
     */
    bool tick(int cycle);

    /** @return true while a request is being processed. */
    bool isBusy() const;

    /** @return The server's identifier. */
    int id() const;

    /** @return Number of requests completed by this server. */
    int requestsHandled() const;

    /** @return Total cycles this server spent busy. */
    long long busyCycles() const;

    /** @return The request currently assigned (valid only while isBusy()). */
    const Request& current() const;

    /** @return Clock value at which the current request will finish. */
    int finishCycle() const;

private:
    int       id_;
    bool      busy_ = false;
    Request   current_;
    int       remaining_ = 0;    ///< Cycles of work left on the current request.
    int       finishCycle_ = 0;
    int       handled_ = 0;
    long long busyCycles_ = 0;
};

#endif // AGF_WEB_SERVER_H
