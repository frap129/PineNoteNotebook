//
// Created by mulliken on 5/22/22.
//
#ifndef CONST_H
#define CONST_H

enum pen_action {
    PEN_UP,
    PEN_DOWN,
    PEN_MOVE,
    PEN_NO_ACTION
};

struct pen_event_t {
    unsigned int x;
    unsigned int y;
    unsigned int pressure;
    pen_action action;
};

// EINK Framebuffer
#define EINK_FB_SIZE        0x400000 /* 4M */

/*
 * ebc system ioctl commands
 */
#define EBC_GET_BUFFER            (0x7000)
#define EBC_SEND_BUFFER            (0x7001)
#define EBC_GET_BUFFER_INFO        (0x7002)
#define EBC_SET_FULL_MODE_NUM    (0x7003)
#define EBC_ENABLE_OVERLAY        (0x7004)
#define EBC_DISABLE_OVERLAY        (0x7005)
#define EBC_GET_OSD_BUFFER        (0x7006)
#define EBC_SEND_OSD_BUFFER        (0x7007)

enum panel_refresh_mode {
    EPD_AUTO = 0,
    EPD_OVERLAY = 1,
    EPD_FULL_GC16 = 2,
    EPD_FULL_GL16 = 3,
    EPD_FULL_GLR16 = 4,
    EPD_FULL_GLD16 = 5,
    EPD_FULL_GCC16 = 6,
    EPD_PART_GC16 = 7,
    EPD_PART_GL16 = 8,
    EPD_PART_GLR16 = 9,
    EPD_PART_GLD16 = 10,
    EPD_PART_GCC16 = 11,
    EPD_A2 = 12,
    EPD_DU = 13,
    EPD_DU4 = 14,
    EPD_A2_ENTER = 15,
    EPD_RESET = 16,
};

struct ebc_buf_info_t {
    int offset;
    int epd_mode;
    int height;
    int width;
    int panel_color;
    int win_x1;
    int win_y1;
    int win_x2;
    int win_y2;
    int width_mm;
    int height_mm;
};

#define LOG_TAG "libpinenotepen"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,__VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#endif //CONST_H