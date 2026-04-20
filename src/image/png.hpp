#pragma once

#include <cstdint>
#include <vector>

namespace png
{
    struct RGB
    {
        uint8_t r, g, b;
    };

    struct FileHeader
    {
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        uint16_t bits_per_pixel;
        uint32_t offset;
    };

    FileHeader read_header(const std::vector<unsigned char> &binary);
    std::vector<unsigned char> read_plte(const std::vector<unsigned char> &binary, uint8_t channels, int length);
    std::vector<unsigned char> read_idat(const std::vector<unsigned char> &binary);
    std::vector<unsigned char> read_iend(const std::vector<unsigned char> &binary);
    std::vector<unsigned char>
    decode(std::vector<unsigned char> &image_binary);
}