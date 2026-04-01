#pragma once

#include <cstdint>
#include <vector>

namespace png
{
    struct FileHeader
    {
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        uint16_t bitsPerPixel;
        uint32_t offset;
    };

    FileHeader readHeader(const std::vector<unsigned char> &binary);
    std::vector<unsigned char> readPLTE(const std::vector<unsigned char> &binary);
    std::vector<unsigned char> readIDAT(const std::vector<unsigned char> &binary);
    std::vector<unsigned char> readIEND(const std::vector<unsigned char> &binary);
    std::vector<unsigned char>
    readImageData(std::vector<unsigned char> &imageBinary);
}