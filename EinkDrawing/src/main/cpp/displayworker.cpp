//
// Created by mulliken on 5/17/22.
//
#include "displayworker.h"
#include <cmath>

DisplayWorker::DisplayWorker(InputState* state) {
    ALOGD("DisplayWorker::DisplayWorker()");

    // Get a pointer to the PineNoteLib
    mPineNoteLib = PineNoteLib::getInstance();

    // Create the thread
    display_thread = std::thread(&DisplayWorker::run, this);

    mState = state;
}

DisplayWorker::~DisplayWorker() {
    ALOGD("DisplayWorker::~DisplayWorker()");
    pen_event_t event{};
    event.action = EXIT_WORKER;
    onPenEvent(event);

    if (display_thread.joinable()) {
        display_thread.join();
    }
}

void DisplayWorker::onPenEvent(pen_event_t pen_event) {
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
        pen_event_t event = equeue.front();
        equeue.pop();
        lck.unlock();

        ALOGD("Pen pressure: %u", event.pressure);

        if (event.action == EXIT_WORKER)
            break;

        if (event.action == PEN_NOTHING)
            continue;

        if (event.action == PEN_DOWN) {
            Point p{event.x, event.y};
            Circle circle(p, getPenRadius(event.pressure));

            mPineNoteLib->drawShape(circle, mState->inputColor);

            prev_event = event;
        } else if (event.action == PEN_MOVE) {
            if (prev_event.x == 0 && prev_event.y == 0) {
                prev_event = event;
                continue;
            }

            if (event.x == prev_event.x && event.y == prev_event.y)
                continue;

            Point p{prev_event.x, prev_event.y};
            Point p2{event.x, event.y};
            LineSegment line(p, p2);
            for (auto point : line.as_points()) {
                Circle circle(point, getPenRadius(event.pressure));
                mPineNoteLib->drawShape(circle, mState->inputColor);
            }

            prev_event = event;
        } else if (event.action == PEN_UP) {
            Point p{prev_event.x, prev_event.y};
            Circle circle(p, getPenRadius(event.pressure));

            mPineNoteLib->drawShape(circle, mState->inputColor);

            prev_event.x = 0;
            prev_event.y = 0;
        }

        mPineNoteLib->sendOsdBuffer();
    }
}

unsigned int DisplayWorker::getPenRadius(unsigned int pressure) {
    int radius = log((double) pressure / PEN_PRESSURE_SCALE) * mState->inputWidth;
    if (radius < 1) {
        radius = mState->inputWidth;
    }
    return radius;
}
