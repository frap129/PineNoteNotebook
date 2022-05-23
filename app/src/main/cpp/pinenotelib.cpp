#include <fcntl.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "pinenotelib.h"

PineNoteLib *PineNoteLib::instance = nullptr;

PineNoteLib *PineNoteLib::getInstance() {
    if (instance == nullptr) {
        instance = new PineNoteLib();
    }

    return instance;
}

PineNoteLib::PineNoteLib() {
    ALOGD("PineNoteLib::PineNoteLib()");
    // Open the device
    openEbc();

    // Map the ebc
    mapEbc();

    // Get the buffer info
    getBufferInfo();

    // Get the EBC buffer
    getEbcBuffer();
    // Get a pointer to the base of the EBC bitmap
    ebc_buffer_base = reinterpret_cast<void *>(
            reinterpret_cast<uintmax_t>(ebc_base) + ebc_buffer->offset);

    // Get the OSD buffer
    getOsdBuffer();
    // Get a pointer to the base of the OSD bitmap
    osd_buffer_base = reinterpret_cast<void *>(
            reinterpret_cast<uintmax_t>(ebc_base) + osd_buffer->offset);
}

PineNoteLib::~PineNoteLib() {
    ALOGD("PineNoteLib::~PineNoteLib()");

    // Unmap the ebc
    unmapEbc();

    // Close the device
    closeEbc();
}

void PineNoteLib::enableOverlay() const {
    if (ioctl(ebc_fd, EBC_ENABLE_OVERLAY, nullptr) != 0) {
        ALOGE("PineNoteLib::enableOverlay(): Failed to enable EBC overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to enable EBC overlay");
    }
}

void PineNoteLib::disableOverlay() {
    clearOverlay();
    if (ioctl(ebc_fd, EBC_DISABLE_OVERLAY, nullptr) != 0) {
        ALOGE("PineNoteLib::disableOverlay(): Failed to disable EBC overlay: %s",
              strerror(errno));
        throw std::runtime_error("Failed to disable EBC overlay");
    }
}

void PineNoteLib::clearOverlay() {
    std::lock_guard<std::mutex> lock(display_mutex);

    clearOsdBuffer();
    sendOsdBuffer();
}

void PineNoteLib::setDisplayMode(panel_refresh_mode mode) {
    ALOGD("PineNoteLib::setDisplayMode(): Setting display mode to %d", mode);
    std::lock_guard<std::mutex> lock(display_mutex);
    ALOGD("PineNoteLib::setDisplayMode(): Lock acquired");

    ALOGD("PineNoteLib::setDisplayMode(): Setting the display mode");
    ebc_buffer->epd_mode = mode;
    ALOGD("PineNoteLib::setDisplayMode(): Done setting mode on EBC Buffer");
    ALOGD("PineNoteLib::setDisplayMode(): Sending EBC Buffer");
    sendEbcBuffer();
    ALOGD("PineNoteLib::setDisplayMode(): Done sending EBC Buffer");
}

void PineNoteLib::sendOsdBuffer() {
    if (ioctl(ebc_fd, EBC_SEND_OSD_BUFFER, &osd_buffer) != 0) {
        ALOGE("PineNoteLib::sendOsdBuffer(): Failed to update EBC overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to update EBC overlay");
    }
    getOsdBuffer();
}

void PineNoteLib::sendEbcBuffer() {
    if (ioctl(ebc_fd, EBC_SEND_BUFFER, &ebc_buffer) != 0) {
        ALOGE("PineNoteLib::sendEbcBuffer(): Failed to send EBC buffer: %s", strerror(errno));
        throw std::runtime_error("Failed to send EBC buffer");
    }
    getEbcBuffer();
}

/*
 * Private functions
 */

void PineNoteLib::openEbc() {
    // Open the device
    ebc_fd = open("/dev/ebc", O_RDWR);
    if (ebc_fd < 0) {
        ALOGE("PineNoteLib::PineNoteLib(): Failed to open /dev/ebc: %s", strerror(errno));
        throw std::runtime_error("Failed to open /dev/ebc");
    }
}

void PineNoteLib::closeEbc() const {
    if (close(ebc_fd) != 0) {
        ALOGE("PineNoteLib::closeEbc(): Failed to close /dev/ebc: %s", strerror(errno));
        throw std::runtime_error("Failed to close /dev/ebc");
    }
}

void PineNoteLib::getBufferInfo() {
    if (ioctl(ebc_fd, EBC_GET_BUFFER_INFO, &ebc_info) != 0) {
        ALOGE("PineNoteLib::getBufferInfo(): Failed to get EBC buffer info: %s", strerror(errno));
        throw std::runtime_error("Failed to get EBC buffer info");
    }
}

void PineNoteLib::mapEbc() {
    // Map the ebc to memory
    // We map EINK_FB_SIZE * 4 bytes, this is because we need to map the EBC bitmap and the OSD
    // bitmap. Each pixel requires 2 bytes (16 bits).
    ebc_base = mmap(nullptr, EINK_FB_SIZE * 4, PROT_READ | PROT_WRITE,
                    MAP_SHARED, ebc_fd, 0);
    if (ebc_base == MAP_FAILED) {
        ALOGE("PineNoteLib::mapEbc() mmap failed with error: %s", strerror(errno));
        throw std::runtime_error("PineNoteLib::mapEbc() mmap failed with error: " +
                                 std::string(strerror(errno)));
    }
}

void PineNoteLib::unmapEbc() const {
    if (munmap(ebc_base, EINK_FB_SIZE * 4) != 0) {
        ALOGE("PineNoteLib::unmapEbc(): Failed to unmap EBC: %s", strerror(errno));
        throw std::runtime_error("Failed to unmap EBC");
    }
}

void PineNoteLib::getEbcBuffer() {
    // Get the EBC buffer
    if (ioctl(ebc_fd, EBC_GET_BUFFER, &ebc_buffer) != 0) {
        ALOGE("PineNoteLib::getEbcBuffer(): Failed to get EBC buffer: %s", strerror(errno));
        throw std::runtime_error("Failed to get EBC buffer");
    }
}

void PineNoteLib::getOsdBuffer() {
    // Get the OSD buffer
    if (ioctl(ebc_fd, EBC_GET_OSD_BUFFER, &osd_buffer) != 0) {
        ALOGE("PineNoteLib::PineNoteLib(): Failed to get OSD buffer: %s", strerror(errno));
        throw std::runtime_error("Failed to get OSD buffer");
    }
}

void PineNoteLib::clearEbcBuffer() const {
    if (memset(ebc_buffer_base, 0xff, ebc_info->width * ebc_info->height * 2) == nullptr) {
        ALOGE("PineNoteLib::clearEbcBuffer(): Failed to clear EBC buffer: %s", strerror(errno));
        throw std::runtime_error("Failed to clear EBC buffer");
    }
}

void PineNoteLib::clearOsdBuffer() const {
    if (memset(osd_buffer_base, 0xff,ebc_info->width * ebc_info->height >> 1) == nullptr) {
        ALOGE("PineNoteLib::clearOverlay(): Failed to clear overlay: %s", strerror(errno));
        throw std::runtime_error("Failed to clear overlay");
    }
}
