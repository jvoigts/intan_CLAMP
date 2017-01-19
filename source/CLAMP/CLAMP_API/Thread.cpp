#include "Thread.h"
#include "common.h"

Thread::Thread() :
    keepGoing(true),
    thread(nullptr)
{
}

Thread::~Thread() {
    close();
}

void callFromThread(Thread* threadWrapper) {
    threadWrapper->run();
    threadWrapper->done();
}

/// Start the thread
void Thread::start() {
    close();
    keepGoing = true;
    thread = new std::thread(callFromThread, this);
}

/** \brief Tell the thread to stop.  
 * 
 *  It notifies the thread to stop.  Subclasses can check the keepGoing flag and end early if it becomes false.
 *
 *  *This function does not do a hard stop on the thread.*
 */
void Thread::stop() {
    keepGoing = false;
}

/// Tell the thread to stop and wait until it does.
void Thread::close() {
    keepGoing = false;
    if (thread) {
        thread->join();
        delete thread;
        thread = nullptr;
    }
}
