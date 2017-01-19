#pragma once

#include <memory>
#include <QObject>
#include "Constants.h"
#include "DataStore.h"
#include "MVC.h"

namespace CLAMP {
    class Board;
}

class Thread;

class GlobalState : public QObject {
    Q_OBJECT

public:
    std::unique_ptr<CLAMP::Board> board;
	DataStore datastore[CLAMP::MAX_NUM_CHIPS];
    std::unique_ptr<Thread> backgroundThread;
    std::unique_ptr<Thread> ledThread;
    double pipetteOffsetInmV[CLAMP::MAX_NUM_CHIPS];
    BoolHolder pipetteOffsetEnabled[CLAMP::MAX_NUM_CHIPS];
	bool saveAuxMode;
	bool vClampX2mode;

    GlobalState(std::unique_ptr<CLAMP::Board>& board_);
    ~GlobalState();

    void threadDone();
    void stateMessage(int unit, const char* message);
    void closeThreads();
    void stopThreads();
    void runThread(Thread* thread);
    void preemptThread(Thread* thread);
    bool isRunning() { return running; }
	void setPipetteOffset(int unit, double value);

signals:
    void pipetteOffsetChanged(int unit, double value);
    void threadStatusChanged(bool running);
    void threadFinished();
    void error(const char* title, const char* message);
    void statusMessage(int unit, QString message); // Note: should not be QString&

public slots:
    void errorMessage(const char* title, const char* message);

private slots:
    void finishThread();

private:
    bool running;
    std::unique_ptr<Thread> preemptedThread;
    std::unique_ptr<Thread> waitingThread;
};
