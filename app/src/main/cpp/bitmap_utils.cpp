//
// Created by Joshua Mulliken on 6/6/22.
//

#include "bitmap_utils.h"

using namespace std;

BitmapImage::~BitmapImage() {
    if (colorTable != nullptr) {
        delete[] colorTable;
        colorTable = nullptr;
    }

    if (bitmapBuffer != nullptr) {
        delete[] bitmapBuffer;
        bitmapBuffer = nullptr;
    }
}

void BitmapImage::create(int32_t width, int32_t height, uint16_t bits_per_pixel, int pixels_per_inch) {
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.bitsPerPixel = bits_per_pixel;
    infoHeader.horizontalResolution = ceil(pixels_per_inch / 0.0254);
    infoHeader.verticalResolution = ceil(pixels_per_inch / 0.0254);

    bitmapBufferSize = bytesPerRow() * infoHeader.height;
    bitmapBuffer = new char[bitmapBufferSize];

    fileHeader.imageDataOffset = sizeof(fileHeader) + sizeof(infoHeader);

    if (infoHeader.bitsPerPixel <= 8) {
        infoHeader.colors = narrow_cast<uint32_t , double>(pow(2, infoHeader.bitsPerPixel));
        colorTableSize = infoHeader.colors * sizeof(RGBA32);
        colorTable = new RGBA32[infoHeader.colors];
        for (auto color = 0; color < infoHeader.colors; color++) {
            uint8_t colorInt = (color << 4) | color;
            colorTable[color] = {
                    colorInt,
                    colorInt,
                    colorInt,
                    0
            };
        }

        fileHeader.imageDataOffset += colorTableSize;
    }

    fileHeader.fileSize = fileHeader.imageDataOffset + bitmapBufferSize;
}

void BitmapImage::read(const char *filename) {
    ifstream input(filename, ios::binary);
    input.read((char *) &fileHeader, sizeof(fileHeader));
    input.read((char *) &infoHeader, sizeof(infoHeader));

    if (infoHeader.colors == 0)
        infoHeader.colors = narrow_cast<uint32_t, double>(pow(2, infoHeader.bitsPerPixel));

    if (infoHeader.bitsPerPixel <= 8) { // We MUST read a colorTable
        colorTable = new RGBA32[infoHeader.colors];
        colorTableSize = infoHeader.colors * sizeof(RGBA32); // Record the colorTableSize
        input.read((char *) colorTable, narrow_cast<int, size_t>(colorTableSize)); // Read the colorTable
    } else if (infoHeader.bitsPerPixel != 24)
        throw runtime_error("only support 1, 4, 8, & 24bit colors");

    // Read the pixel data
    bitmapBufferSize = bytesPerRow() * infoHeader.height;
    bitmapBuffer = new char[bitmapBufferSize];
    input.read((char *) bitmapBuffer, narrow_cast<int, size_t>(bitmapBufferSize));

    input.close();
}

void BitmapImage::write(const char *filename) {
    ofstream output(filename, ios::binary);
    output.write((char *) &fileHeader, sizeof(fileHeader));
    output.write((char *) &infoHeader, sizeof(infoHeader));

    if (infoHeader.bitsPerPixel <= 8) {
        output.write((char *) colorTable, narrow_cast<int, size_t>(colorTableSize));
    } else if (infoHeader.bitsPerPixel != 24)
        throw runtime_error("only support 1, 4, 8, & 24bit colors");

    output.write((char *) bitmapBuffer, narrow_cast<int, size_t>(bitmapBufferSize));

    output.close();
}

unsigned int BitmapImage::bytesPerRow() const {
    unsigned int bits_per_row = infoHeader.width * infoHeader.bitsPerPixel;
    unsigned int bytes_per_row = bits_per_row / 8;

    // Align the pixel data to 4 bytes
    while (bytes_per_row % 4 != 0) {
        bytes_per_row++;
    }
    return bytes_per_row;
}