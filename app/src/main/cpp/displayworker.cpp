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

    pen_event_queue_mutex.lock();
    pen_event_queue.push(pen_event);
    pen_event_queue_mutex.unlock();
}

void DisplayWorker::run() {
    ALOGD("DisplayWorker::run()");
    while (true) {
        // TODO: Implement a blocking queue
        pen_event_queue_mutex.lock();
        if (pen_event_queue.empty()) {
            pen_event_queue_mutex.unlock();
            continue;
        }

        pen_event_t *event = pen_event_queue.front();
        pen_event_queue.pop();
        pen_event_queue_mutex.unlock();

        if (event->action == EXIT_WORKER) {
            break;
        }

        if (event->action == PEN_MOVE) {
            mPineNoteLib->drawPoint(event->x, event->y, PEN_RADIUS, PEN_COLOR);
        }
    }
}
