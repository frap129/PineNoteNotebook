#ifndef PENWORKER_H
#define PENWORKER_H

#include "pinenotelib.h"
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

class PenWorker {
public:
    PenWorker();

    ~PenWorker();

    void registerListener(const std::function<void(pen_event_t *)> &listener);

private:
    std::mutex listener_mutex;
    std::vector<std::function<void(pen_event_t *)>> listeners;
    std::atomic<bool> shouldStop{};
    std::thread penWorkerThread;
    pen_event_t *pen_state{};

    void run(const std::string &handlerPath);
};

#endif //PENWORKER_H