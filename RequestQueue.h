/**
 * @file RequestQueue.h
 * @brief FIFO queue of Request objects with a peak-depth counter.
 */
#ifndef AGF_REQUEST_QUEUE_H
#define AGF_REQUEST_QUEUE_H

#include <cstddef>
#include <queue>

#include "Request.h"

/**
 * @class RequestQueue
 * @brief First-in-first-out queue of pending requests.
 *
 * Wraps @c std::queue<Request> so the LoadBalancer never depends on the
 * container choice and so the peak queue depth can be recorded for the
 * end-of-run summary. FIFO guarantees that an old request is never starved
 * by a newer one.
 */
class RequestQueue {
public:
    /**
     * @brief Append a request to the back of the queue.
     * @param r The request to enqueue.
     */
    void push(const Request& r);

    /**
     * @brief Remove and return the request at the front.
     * @pre !empty()
     * @return The oldest queued request.
     */
    Request pop();

    /**
     * @brief Peek at the oldest request without removing it.
     * @pre !empty()
     * @return Const reference to the front request.
     */
    const Request& front() const;

    /** @return true when no requests are waiting. */
    bool empty() const;

    /** @return Number of requests currently waiting. */
    std::size_t size() const;

    /** @return The largest size the queue has reached since construction. */
    std::size_t peakSize() const;

private:
    std::queue<Request> q_;      ///< Underlying FIFO container.
    std::size_t peak_ = 0;       ///< Highest observed depth.
};

#endif // AGF_REQUEST_QUEUE_H
