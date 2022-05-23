//
// Created by mulliken on 5/17/22.
//
#include "displayworker.h"

DisplayWorker::DisplayWorker(PineNoteLib *lib) {
    ALOGD("DisplayWorker::DisplayWorker()");
    pnl = lib;

    // Enable the EBC overlay
    pnl->enableOverlay();

    // Create the thread
    display_thread = std::thread(&DisplayWorker::run, this);
}

DisplayWorker::~DisplayWorker() {
    ALOGD("DisplayWorker::~DisplayWorker()");
    should_stop = true;
    if (display_thread.joinable()) {
        display_thread.join();
    }

    // Disable the EBC overlay
    pnl->disableOverlay();
}

void DisplayWorker::run() {
    ALOGD("DisplayWorker::run(): Starting display thread");
    while (!should_stop) {
        // Get the latest pen event
        pen_event_queue_mutex.lock();

        if (pen_event_queue.empty()) {
            pen_event_queue_mutex.unlock();
            continue;
        }

        auto pen_event = pen_event_queue.front();
        pen_event_queue.pop();
        pen_event_queue_mutex.unlock();

        // Create a new buffer info struct
        // From my understanding the EBC_SEND_OSD_BUFFER ioctl will set the pixels to the value of
        // the corresponding pixel in the display buffer.
        // It seems to do partial updates, so we can set win_x1, win_y1, win_x2, win_y2 to the
        // area we want to update.

        // Lets start by trying to set a small square (about the size of the pen point)
        // This will be the area we update

        // If the pen event is a pen move, we want to write a 10x10 square
        if (pen_event->action == PEN_MOVE) {
            std::lock_guard<std::mutex> lock(pnl->display_mutex);

            // We need black in the buffer so that it will write black to the screen
            // We can set the color to black by setting the alpha to 0

            pnl->osd_buffer->win_x1 = (int) pen_event->x - 2;
            pnl->osd_buffer->win_y1 = (int) pen_event->y - 2;
            pnl->osd_buffer->win_x2 = (int) pen_event->x + 2;
            pnl->osd_buffer->win_y2 = (int) pen_event->y + 2;

            // Get the offset into the display buffer
            // There are two bytes per pixel, so we need to do a >> 1 once we get the pixel

            // The first pixel is at (win_x1, win_y1)
            // The last pixel is at (win_x2, win_y2)
            // We need to make sure that we do not set the pixels outside of that rectangle
            for (int x = pnl->ebc_info->win_x1;
                 x <= pnl->ebc_info->win_x2; x++) {
                for (int y = pnl->ebc_info->win_y1;
                     y <= pnl->ebc_info->win_y2; y++) {
                    // Get the offset into the display buffer
                    uint8_t *display_buffer_offset =
                            (uint8_t *) pnl->osd_buffer_base +
                            ((y * pnl->ebc_info->width + x) >> 1);

                    // Set the pixel to black
                    memset(display_buffer_offset, 0x0f, 2);
                }
            }

            // Write the buffer to the screen
            pnl->sendOsdBuffer();
        }
    }
}

void DisplayWorker::addPenEvent(pen_event_t *pen_event) {
    pen_event_queue_mutex.lock();
    pen_event_queue.push(pen_event);
    pen_event_queue_mutex.unlock();
}