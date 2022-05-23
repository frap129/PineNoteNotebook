#include "penworker.h"

PenWorker::PenWorker() {
    // Open the file
    std::ifstream inputFile("/proc/bus/input/devices");
    if (!inputFile.good()) {
        ALOGE("Failed to open /proc/bus/input/devices");
        throw std::runtime_error("Failed to open /proc/bus/input/devices");
    }

    std::string handler;
    std::string line;
    while (std::getline(inputFile, line)) {
        // run until we find the line with both the vendor and product info
        if (line.find("Vendor=056a") != std::string::npos &&
            line.find("Product=0000") != std::string::npos) {
            // Let's double-check we have the right device by checking the name
            std::getline(inputFile, line);
            if (line.find("Name=\"Wacom I2C Digitizer\"") != std::string::npos) {
                // We have the right device, so let's get the handler
                while (std::getline(inputFile, line)) {
                    if (line.find("H: Handlers=") != std::string::npos) {
                        // We have the handler, so let's get the device number
                        handler = line.substr(12, 6);
                        break;
                    }
                }
            }
        }
    }

    // Close the file
    inputFile.close();

    if (handler.empty()) {
        ALOGE("Failed to find the pen handler");
        throw std::runtime_error("Failed to find the pen handler");
    }

    // Start the polling thread
    penWorkerThread = std::thread(&PenWorker::run, this, "/dev/input/" + handler);
}

PenWorker::~PenWorker() {
    shouldStop = true;
    if (penWorkerThread.joinable()) {
        penWorkerThread.join();
    }
}

void PenWorker::run(const std::string &handlerPath) {
    ALOGD("PenWorker::run(): Starting pen worker thread");

    // Open the handler
    std::ifstream fh(handlerPath, std::ios::binary);
    if (!fh.good()) {
        ALOGE("PenWorker::run(): Failed to open %s", handlerPath.c_str());
        ALOGE("PenWorker::run(): Error code: %s", strerror(errno));
        throw std::runtime_error("Failed to open pen handler");
    }

    while (true) {
        if (shouldStop)
            break;

        // Read the event
        struct input_event event{};
        fh.read(reinterpret_cast<char *>(&event), sizeof(event));
        if (fh.gcount() != sizeof(event)) {
            ALOGE("PenWorker::run(): Failed to read event. Error: %s", strerror(errno));
            throw std::runtime_error("Failed to read event");
        }

        // Check the event type
        switch (event.type) {
            case EV_SYN:
                if ((pen_state->action == PEN_DOWN || pen_state->action == PEN_MOVE)
                    && pen_state->pressure >= 100) {
                    pen_state->action = PEN_MOVE;
                }

                listener_mutex.lock();
                for (auto &listener: listeners) {
                    // Send the data to the listener
                    auto *listener_event = new pen_event_t;
                    listener_event->x = pen_state->x;
                    listener_event->y = pen_state->y;
                    listener_event->pressure = pen_state->pressure;
                    listener_event->action = pen_state->action;
                    listener(listener_event);
                }
                listener_mutex.unlock();

                break;
            case EV_ABS:
                switch (event.code) {
                    case ABS_X:
                        pen_state->x = event.value;
                        break;
                    case ABS_Y:
                        pen_state->y = event.value;
                        break;
                    case ABS_PRESSURE:
                        pen_state->pressure = event.value;
                        break;
                    default:
                        break;
                }
                break;
            case EV_KEY:
                switch (event.code) {
                    case BTN_TOUCH:
                        pen_state->action = pen_action::PEN_DOWN;
                        break;
                    case BTN_TOOL_PEN:
                        pen_state->action = pen_action::PEN_UP;
                        break;
                    default:
                        pen_state->action = pen_action::PEN_NO_ACTION;
                        break;
                }
                break;
        }
    }

    // Close the handler
    fh.close();
}

void PenWorker::registerListener(const std::function<void(pen_event_t *)> &listener) {
    ALOGD("PenWorker::registerListener(): Registering listener");

    listener_mutex.lock();
    listeners.push_back(listener);
    listener_mutex.unlock();
}
