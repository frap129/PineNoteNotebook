//
// Created by mulliken on 5/17/22.
//
#ifndef PINENOTELIB_H
#define PINENOTELIB_H

#include "const.h"
#include "raster_utils.h"
#include "bitmap_utils.h"

#include <mutex>
#include <math.h>

#define EBC_DEVICE "/dev/ebc"

class PineNoteLib {
public:

    static PineNoteLib *getInstance();

    static void destroyInstance();

    ~PineNoteLib();

    void dumpToBitmap(const char * filename) const;

    void setDrawArea(int x1, int y1, int x2, int y2);

    void clearOverlay();

    void enableOverlay() ;

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

    // The EBC_GET_BUFFER_INFO call returns info about the screen
    // The following values can be trusted from this call:
    // ebc_info->width
    // ebc_info->height
    // ebc_info->panel_color
    // ebc_info->epd_mode
    ebc_buf_info_t ebc_info{};

    // pointer to where the OSD bitmap buffer is mapped in memory
    uint8_t *osd_buffer_base{};
    // The EBC_GET_OSD_BUFFER call returns info about the OSD buffer
    // The following values can be trusted from this call:
    // osd_buffer->offset // This is the offset from ebc_base where the OSD bitmap buffer starts
    ebc_buf_info_t osd_buffer{};

    void flashOverlay();

    void drawPixel(uint x, uint y, uint8_t color) const;

    void drawShape(Shape& shape, unsigned int color) const;

    uint32_t *getFullPixelData() const;

    // The area of the screen that is being drawn to
    int display_x1{};
    int display_y1{};
    int display_x2{};
    int display_y2{};

    uint32_t *getBoundedPixelData() const;

private:
    static PineNoteLib *instance;

    PineNoteLib();

    void getOsdBuffer();

    void clearOsdBuffer() const;

    // pointer to where the EBC is mapped in memory
    uint8_t *ebc_base = nullptr;

    uint32_t getArgbPixelAt(unsigned int offset, int x, int y) const;
};

#endif // PINENOTELIB_H