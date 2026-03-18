#pragma once

#include <cstdint>
#include <vector>

namespace qoi
{
    struct FileHeader
    {
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        uint8_t colorspace;
        uint32_t offset;
    };

    FileHeader readHeader(const std::vector<unsigned char> &binary);
    std::vector<unsigned char>
    readImageData(std::vector<unsigned char> &imageBinary);

}