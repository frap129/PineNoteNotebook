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

class PenWorker {
public:
    PenWorker();

    ~PenWorker();

    void registerListener(const std::function<void(pen_event_t *)> &listener);

private:
    std::mutex listener_mutex;
    std::vector<std::function<void(pen_event_t *)>> listeners;
    std::thread penWorkerThread;
    fd_set fds{};
    int event_fd;
    int pipefd[2]{};

    void run();
};

#endif //PENWORKER_H