#ifndef PENWORKER_H
#define PENWORKER_H

#include "const.h"

#include <iostream>
#include <atomic>
#include <functional>
#include <mutex>
#include <vector>
#include <fstream>
#include <linux/input.h>
#include <fcntl.h>
#include <android/log.h>
#include <thread>
#include <unistd.h>

using namespace std;

class PenWorker {
public:
    PenWorker();

    ~PenWorker();

    void registerListener(const function<void(pen_event_t *)> &listener);

private:
    mutex listener_mutex;
    vector<function<void(pen_event_t *)>> listeners;
    thread penWorkerThread;
    fd_set fds{};
    int event_fd;
    int pipefd[2]{};

    unsigned int prev_pressure = 0;

    void run();
};

#endif //PENWORKER_H