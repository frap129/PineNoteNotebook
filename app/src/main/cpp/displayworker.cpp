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

        if (event->action == PEN_DOWN) {
            Point p{event->x, event->y};
            Circle circle(p, PEN_RADIUS);

            mPineNoteLib->drawShape(circle, PEN_COLOR);

            prev_event = *event;
        }

        if (event->action == PEN_MOVE) {
            Point prev_p{prev_event.x, prev_event.y};
            Point p{event->x, event->y};

            LineSegment line(prev_p, p);

            Rectangle rectangle(&line, PEN_RADIUS);

            mPineNoteLib->drawShape(rectangle, PEN_COLOR);

            prev_event = *event;
        }

        if (event->action == PEN_UP) {
            Point p{event->x, event->y};
            Circle circle(p, PEN_RADIUS);

            mPineNoteLib->drawShape(circle, PEN_COLOR);
        }
    }
}
