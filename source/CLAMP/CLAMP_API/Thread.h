#pragma once

#include <thread>

/** \brief Class-based wrapper for threads
 *
 *  The standard library has a function-based approach to threads, which makes it hard to pass state around.
 *
 *  To implement a thread with some kind of internal state, make a subclass of this class and implement the
 *  run method.
 */
class Thread {
public:
    Thread();
    virtual ~Thread();
    void start();
    void stop();
    void close();
    /// Implement this in a subclass to do the thread's logic
    virtual void run() {}
    /// Implement this in a subclass to do anything the thread needs to do when it finishes
    virtual void done() {}

protected:
    /** \brief Subclasses that do long computations can check this periodically; it's set to false when the thread is requested to stop.
     *
     *  For example:
     * \code
        while (keepGoing) {
          // do some calcuations
        }
     * \endcode
    */
    volatile bool keepGoing;

private:
    std::thread* thread;
};
