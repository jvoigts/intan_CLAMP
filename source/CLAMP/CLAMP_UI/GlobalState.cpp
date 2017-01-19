#include "Constants.h"
#include "GlobalState.h"
#include "Board.h"
#include "Thread.h"
#include "ClampThread.h"

using CLAMP::Board;
using std::unique_ptr;

//--------------------------------------------------------------------------
class LEDThread : public Thread {
public:
    LEDThread(Board& b) : board(b) {}

    void run() override {
        uint8_t value = 1;
        while (keepGoing) {
            board.setFpgaLeds(value);
            value = (value & 0x80) ? 1 : (value << 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    void done() override {
        board.setFpgaLeds(0);
    }

private:
    Board& board;
};

//--------------------------------------------------------------------------
GlobalState::GlobalState(std::unique_ptr<CLAMP::Board>& board_) :
    board(std::move(board_)),
    running(false)
{
	saveAuxMode = true;
	vClampX2mode = false;
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		datastore[i].state = this;
		pipetteOffsetEnabled[i].setValue(true);
	}
    connect(this, SIGNAL(threadFinished()), this, SLOT(finishThread()));
    ledThread.reset(new LEDThread(*board));
}

GlobalState::~GlobalState() {
}

void GlobalState::stateMessage(int unit, const char* message) {
    emit statusMessage(unit, message);
}

void GlobalState::runThread(Thread* thread) {
    running = true;
    backgroundThread.reset(thread);

    emit threadStatusChanged(true);
    backgroundThread->start();
    ledThread->start();
}

// Stops the current thread; stores it away
// Runs the input thread
// When the input thread finishes, will return to the thread it was running before all this started
void GlobalState::preemptThread(Thread* thread) {
    closeThreads();
    running = false;

    if (backgroundThread.get() == nullptr) {
        runThread(thread);
    }
    else {
        if (waitingThread.get() == nullptr) {
            waitingThread.reset(thread);
        }
    }
}

void GlobalState::finishThread()
{
    closeThreads();
    running = false;

    if (waitingThread.get() != nullptr) {
        preemptedThread.swap(backgroundThread);
        runThread(waitingThread.release());
    }
    else if (preemptedThread.get() != nullptr) {
        runThread(preemptedThread.release());
    }
    else {
        backgroundThread.reset();
        emit threadStatusChanged(false);
    }
}

// This is a hack - a thread calls threadDone, which emits threadFinished(), which calls finishThread
// The reason we can't do it all at once is that finishThread zaps the calling thread, which would cause grief; 
// the way it's done, you wind up calling finishThread external to the thread that's going away
void GlobalState::threadDone()
{
    emit threadFinished();
}

void GlobalState::setPipetteOffset(int unit, double value) {
    if (pipetteOffsetInmV[unit] != value) {
        pipetteOffsetInmV[unit] = value;
        emit pipetteOffsetChanged(unit, pipetteOffsetInmV[unit]);
    }
}

void GlobalState::errorMessage(const char* title, const char* message) {
    emit error(title, message);
}

void GlobalState::closeThreads() {
    if (backgroundThread.get() != nullptr) {
        backgroundThread->close();
    }
    if (ledThread.get() != nullptr) {
        ledThread->close();
    }
}

void GlobalState::stopThreads() {
    backgroundThread->stop();
    ledThread->stop();
}
