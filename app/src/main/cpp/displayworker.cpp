//
// Created by mulliken on 5/17/22.
//
#include "displayworker.h"

#define PEN_RADIUS 3
#define PEN_COLOR 0x0f

DisplayWorker::DisplayWorker() {
    ALOGD("DisplayWorker::DisplayWorker()");

    // Get a pointer to the PineNoteLib
    mPineNoteLib = PineNoteLib::getInstance();

    // Create the thread
    display_thread = std::thread(&DisplayWorker::run, this);
}

DisplayWorker::~DisplayWorker() {
    ALOGD("DisplayWorker::~DisplayWorker()");
    auto *event = new pen_event_t();
    event->action = EXIT_WORKER;
    onPenEvent(event);

    if (display_thread.joinable()) {
        display_thread.join();
    }
}

void DisplayWorker::onPenEvent(pen_event_t *pen_event) {
//    ALOGD("DisplayWorker::onPenEvent()");

    std::unique_lock<std::mutex> lck{emutex};
    equeue.push(pen_event);
    econd.notify_one();
}

void DisplayWorker::run() {
    ALOGD("DisplayWorker::run()");
    while (true) {
        std::unique_lock<std::mutex> lck{emutex}; // Lock the mutex
        econd.wait(lck, [this] { return !equeue.empty(); }); // Wait for a pen event to be
                                                                     // added to the queue
        pen_event_t *event = equeue.front();
        equeue.pop();
        lck.unlock();

        if (event->action == EXIT_WORKER) {
            break;
        }

        if (event->action == PEN_MOVE) {
            mPineNoteLib->drawPoint(event->x, event->y, PEN_RADIUS, PEN_COLOR);
        }
    }
}
