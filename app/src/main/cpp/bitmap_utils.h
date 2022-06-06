//
// Created by Joshua Mulliken on 6/6/22.
//

#ifndef PINENOTE_NOTEBOOK_BITMAP_UTILS_H
#define PINENOTE_NOTEBOOK_BITMAP_UTILS_H

#include "utils.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>

#pragma pack(push, 1)
struct BitmapFileHeader { // The full size is 14 bytes
    const char magic[2]{'B', 'M'}; // Should equal 'BM'
    uint32_t fileSize{0}; // Size of the bitmap file in bytes
    const uint16_t reservedOne{0}; // Should be 0
    const uint16_t reservedTwo{0}; // Should be 0
    uint32_t imageDataOffset{0}; // Offset into bitmap file where pixel array can be found
};
#pragma pack(pop)
#pragma pack(push, 1)
struct BitmapInfoHeader { // The full size is 40 bytes
    const uint32_t size{40}; // Size of this header in bytes (should be 40)
    int32_t width{0}; // Width of the image in pixels
    int32_t height{0}; // Height of the image in pixels
    const uint16_t colorPlanes{1}; // MUST be 1
    uint16_t bitsPerPixel{0}; // Number of bits per pixel (AKA color depth)
    const uint32_t compressionMethod{0}; // This program only supports 0. The compression methods can be found here: https://en.wikipedia.org/wiki/BMP_file_format
    const uint32_t imageSize{0}; // We only support value of 0. Size of the raw bitmap data.
    uint32_t horizontalResolution{0}; // In pixels per meter
    uint32_t verticalResolution{0}; // In pixels per meter
    uint32_t colors{0}; // Number of colors in the color palette (if bitdepth is <= 8 then this must be set to 2^n; otherwise it can be 0)
    const uint32_t importantColors{0}; // We only support value of 0. Usually ignored - set to 0
};
#pragma pack(pop)

struct RGBA32 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t zero;
};

class BitmapImage {
public:
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
    RGBA32 *colorTable = nullptr;
    size_t colorTableSize;
    char *bitmapBuffer = nullptr;
    size_t bitmapBufferSize;

    ~BitmapImage();

    void create(int32_t width, int32_t height, uint16_t bits_per_pixel, int pixels_per_inch);

    void read(const char * filename);

    void write(const char * filename);
private:
    unsigned int bytesPerRow() const;
};

#endif //PINENOTE_NOTEBOOK_BITMAP_UTILS_H
