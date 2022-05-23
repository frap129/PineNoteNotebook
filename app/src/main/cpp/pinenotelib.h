//
// Created by mulliken on 5/17/22.
//
#ifndef PINENOTELIB_H
#define PINENOTELIB_H

#include "const.h"

#include <mutex>

class PineNoteLib {
public:
    // Lock for all display operations
    std::mutex display_mutex;

    static PineNoteLib *getInstance();

    ~PineNoteLib();

    /*
     * High level functions
     */
    void setDisplayMode(panel_refresh_mode mode);

    void clearOverlay();

    void enableOverlay() const;

    void disableOverlay();

    /*
     * Low level functions
     */

    // This function tells the EBC to update the overlay with the current OSD buffer
    // Before calling this function you should set the following values:
    // osd_buffer->win_x1
    // osd_buffer->win_y1
    // osd_buffer->win_x2
    // osd_buffer->win_y2
    void sendOsdBuffer();

    // This function tells the EBC to update the screen with the current EBC buffer
    // Before calling this function you should set the following values:
    // ebc_buffer->win_x1
    // ebc_buffer->win_y1
    // ebc_buffer->win_x2
    // ebc_buffer->win_y2
    // ebc_buffer->mode // This should be set to the appropriate mode for your use case
    void sendEbcBuffer();

    // The EBC_GET_BUFFER_INFO call returns info about the screen
    // The following values can be trusted from this call:
    // ebc_info->width
    // ebc_info->height
    // ebc_info->panel_color
    // ebc_info->epd_mode
    ebc_buf_info_t *ebc_info{};

    // pointer to where the EBC bitmap buffer is mapped in memory
    void *ebc_buffer_base{};
    // The EBC_GET_BUFFER call returns info about writing the primary buffer to the screen
    // The following values can be trusted from this call:
    // ebc_buffer->offset // This is the offset from ebc_base where the EBC bitmap buffer starts
    ebc_buf_info_t *ebc_buffer{};

    // pointer to where the OSD bitmap buffer is mapped in memory
    void *osd_buffer_base{};
    // The EBC_GET_OSD_BUFFER call returns info about the OSD buffer
    // The following values can be trusted from this call:
    // osd_buffer->offset // This is the offset from ebc_base where the OSD bitmap buffer starts
    ebc_buf_info_t *osd_buffer{};

private:
    static PineNoteLib *instance;

    PineNoteLib();

    /*
     * Low level functions
     */

    void openEbc();

    void closeEbc() const;

    void getBufferInfo();

    void mapEbc();

    void unmapEbc() const;

    void getEbcBuffer();

    void getOsdBuffer();

    void clearEbcBuffer() const;

    void clearOsdBuffer() const;

    // EBC device framebuffer
    int ebc_fd = -1;
    // pointer to where the EBC is mapped in memory
    void *ebc_base = nullptr;
};

#endif // PINENOTELIB_H