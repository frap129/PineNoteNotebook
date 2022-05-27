#include <fcntl.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "pinenotelib.h"

PineNoteLib *PineNoteLib::instance = nullptr;

PineNoteLib *PineNoteLib::getInstance() {
    ALOGD("PineNoteLib::getInstance()");
    if (instance == nullptr) {
        instance = new PineNoteLib();
    }

    return instance;
}

void PineNoteLib::destroyInstance() {
    ALOGD("PineNoteLib::destroyInstance()");
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

PineNoteLib::PineNoteLib() {
    ALOGD("PineNoteLib::PineNoteLib()");
    // Open the device
    int ebc_fd = open(EBC_DEVICE, O_RDWR, 0);
    if (ebc_fd < 0) {
        ALOGE("%s: failed to open /dev/ebc: %s", __FUNCTION__, strerror(errno));
        throw std::runtime_error("Failed to open /dev/ebc");
    }

    // Map the ebc
    ebc_base = (uint8_t *) mmap(nullptr, EINK_FB_SIZE * 4, PROT_READ | PROT_WRITE,
                    MAP_SHARED, ebc_fd, 0);
    if (ebc_base == MAP_FAILED) {
        ALOGE("%s: mmap failed with error: %s", __FUNCTION__, strerror(errno));
        throw std::runtime_error("Failed to mmap /dev/ebc");
    }

    // Close the current event_fd
    close(ebc_fd);

    ebc_fd = open(EBC_DEVICE, O_RDWR, 0);

    // Get the ebc info
    if (ioctl(ebc_fd, EBC_GET_BUFFER_INFO, &ebc_info) != 0) {
        ALOGE("%s: Failed to get EBC buffer info: %s", __FUNCTION__, strerror(errno));
        throw std::runtime_error("Failed to get EBC buffer info");
    }

    close(ebc_fd);

    // Get the OSD buffer
    getOsdBuffer();
    // Get a pointer to the base of the OSD bitmap
    osd_buffer_base = ebc_base + osd_buffer.offset;
}

PineNoteLib::~PineNoteLib() {
    ALOGD("PineNoteLib::~PineNoteLib()");
    // Unmap the ebc
    if (munmap(ebc_base, EINK_FB_SIZE * 4) != 0) {
        ALOGE("PineNoteLib::unmapEbc(): Failed to unmap EBC: %s", strerror(errno));
    }
}

void PineNoteLib::drawPoint(uint x, uint y, int radius, uint color) {
    uint fromX = x - radius;
    uint fromY = y - radius;
    uint toX = x + radius;
    uint toY = y + radius;

    for (uint xx = fromX; xx <= toX; xx++) {
        for (uint yy = fromY; yy <= toY; yy++) {
            drawPixel(xx, yy, color);
        }
    }

    flashOverlay();
}

void PineNoteLib::setDrawArea(int x1, int y1, int x2, int y2) {
    // TODO: Set the draw area

    display_x1 = x1;
    display_y1 = y1;
    display_x2 = x2;
    display_y2 = y2;
}

void PineNoteLib::enableOverlay() {
    ALOGD("PineNoteLib::enableOverlay()");
    clearOverlay();

    int ebc_fd = open(EBC_DEVICE, O_RDWR, 0);

    if (ioctl(ebc_fd, EBC_ENABLE_OVERLAY, nullptr) != 0) {
        ALOGE("PineNoteLib::enableOverlay(): Failed to enable EBC overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to enable EBC overlay");
    }

    close(ebc_fd);
}

void PineNoteLib::disableOverlay() {
    ALOGD("PineNoteLib::disableOverlay()");
    clearOverlay();

    int ebc_fd = open(EBC_DEVICE, O_RDWR, 0);

    if (ioctl(ebc_fd, EBC_DISABLE_OVERLAY, nullptr) != 0) {
        ALOGE("PineNoteLib::disableOverlay(): Failed to disable EBC overlay: %s",
              strerror(errno));
        throw std::runtime_error("Failed to disable EBC overlay");
    }

    close(ebc_fd);
}

void PineNoteLib::clearOverlay() {
    ALOGD("PineNoteLib::clearOverlay()");
    clearOsdBuffer();

    osd_buffer.win_x1 = 0;
    osd_buffer.win_y1 = 0;
    osd_buffer.win_x2 = ebc_info.width;
    osd_buffer.win_y2 = ebc_info.height;
    osd_buffer.epd_mode = EPD_OVERLAY;

    sendOsdBuffer();
}

void PineNoteLib::sendOsdBuffer() {
//    ALOGD("PineNoteLib::sendOsdBuffer()");
    int ebc_fd = open(EBC_DEVICE, O_RDWR, 0);

    if (ioctl(ebc_fd, EBC_SEND_OSD_BUFFER, &osd_buffer) != 0) {
        ALOGE("PineNoteLib::sendOsdBuffer(): Failed to update EBC overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to update EBC overlay");
    }

    close(ebc_fd);
}

void PineNoteLib::getOsdBuffer() {
    ALOGD("PineNoteLib::getOsdBuffer()");
    int ebc_fd = open(EBC_DEVICE, O_RDWR, 0);

    // Get the OSD buffer
    if (ioctl(ebc_fd, EBC_GET_OSD_BUFFER, &osd_buffer) != 0) {
        ALOGE("PineNoteLib::PineNoteLib(): Failed to get OSD buffer: %s", strerror(errno));
        throw std::runtime_error("Failed to get OSD buffer");
    }

    close(ebc_fd);
}

void PineNoteLib::clearOsdBuffer() const {
    ALOGD("PineNoteLib::clearOsdBuffer()");
    if (memset(osd_buffer_base, 0xff, ebc_info.width * ebc_info.height >> 1) == nullptr) {
        ALOGE("PineNoteLib::clearOverlay(): Failed to clear overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to clear overlay");
    }
}

void PineNoteLib::flashOverlay() {
    osd_buffer.win_x1 = 0;
    osd_buffer.win_y1 = 0;
    osd_buffer.win_x2 = ebc_info.width;
    osd_buffer.win_y2 = ebc_info.height;
    osd_buffer.epd_mode = EPD_OVERLAY;

    sendOsdBuffer();
}

void PineNoteLib::drawPixel(uint x, uint y, uint8_t color) const {
//    ALOGD("PineNoteLib::drawPixel()");
    if (x >= ebc_info.width || y >= ebc_info.height) {
        return;
    }

    if (color > 0x0f) {
        ALOGE("PineNoteLib::drawPixel(): Invalid color: %d", color);
        throw std::runtime_error("Invalid color");
    }

    color = ~color;

    // Try with the y-based offset
    uint offset = (y - 1) * ebc_info.width + (x - 1);

    // Try with the x-based offset
//    uint offset = x * ebc_info.height + y;

    if (offset % 2 == 1) {
        offset -= 1;
        offset = offset / 2;

        osd_buffer_base[offset] |= 0xf0;
        osd_buffer_base[offset] &= color << 4;
    } else {
        offset = offset / 2;

        osd_buffer_base[offset] |= 0x0f;
        osd_buffer_base[offset] &= color;
    }
}
