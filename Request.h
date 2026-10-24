/**
 * @file Request.h
 * @brief Plain-data description of one web request handled by AggieForge Traffic Cloud.
 */
#ifndef AGF_REQUEST_H
#define AGF_REQUEST_H

#include <string>

/**
 * @struct Request
 * @brief One unit of work that a WebServer must process.
 *
 * A request has no behaviour of its own, so it is a struct. The four fields
 * required by the assignment are @c ipIn, @c ipOut, @c time and @c jobType;
 * @c arrivedCycle is added so the balancer can report the average time a
 * request waited in the queue.
 */
struct Request {
    std::string ipIn;      ///< Requester IPv4 address, e.g. "172.16.4.9".
    std::string ipOut;     ///< Destination IPv4 address.
    int  time = 0;         ///< Processing time in clock cycles.
    char jobType = 'P';    ///< 'P' = processing job, 'S' = streaming job.
    int  arrivedCycle = 0; ///< Clock value when the request was created.
};

#endif // AGF_REQUEST_H
