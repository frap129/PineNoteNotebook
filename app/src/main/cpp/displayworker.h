//
// Created by mulliken on 5/17/22.
//
#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include "const.h"
#include "pinenotelib.h"

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

#define PEN_SIZE 2

class DisplayWorker {
public:
    DisplayWorker();

    ~DisplayWorker();

    void onPenEvent(pen_event_t *pen_event);

private:
    PineNoteLib *mPineNoteLib;

    std::queue<pen_event_t *> equeue{};
    std::mutex emutex;
    std::condition_variable econd;

    std::thread display_thread;

    void run();
};

#endif //DISPLAYWORKER_H