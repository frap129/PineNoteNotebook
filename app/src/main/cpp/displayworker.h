//
// Created by mulliken on 5/17/22.
//
#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include "pinenotelib.h"
#include "const.h"

#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <android/log.h>
#include <thread>

class DisplayWorker {
public:
    DisplayWorker(PineNoteLib *lib);

    ~DisplayWorker();

    void addPenEvent(pen_event_t *pen_event);

private:
    PineNoteLib *pnl;

    std::queue<pen_event_t *> pen_event_queue{};
    std::mutex pen_event_queue_mutex;
    std::atomic_bool should_stop{};
    std::thread display_thread;

    void run();
};

#endif //DISPLAYWORKER_H