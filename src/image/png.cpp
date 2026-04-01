#include "png.hpp"
#include <string>

png::FileHeader png::readHeader(const std::vector<unsigned char> &binary)
{
    png::FileHeader header;
    header.width = (binary[16] << 24) | (binary[17] << 16) | (binary[18] << 8) | binary[19];
    header.height = (binary[20] << 24) | (binary[21] << 16) | (binary[22] << 8) | binary[23];

    uint8_t bitDepth = binary[24];
    uint8_t colorType = binary[25];

    switch (colorType)
    {
    case 0:
        header.channels = 1;
        break; // Greyscale
    case 2:
        header.channels = 3;
        break; // Truecolor
    case 3:
        header.channels = 1;
        break; // Indexed-color
    case 4:
        header.channels = 2;
        break; // Greyscale + alpha
    case 6:
        header.channels = 4;
        break; // Truecolor + alpha
    }

    header.bitsPerPixel = bitDepth * header.channels;
    header.offset = 33; // signature(4) + length(4) + IHDR(4) + IHDR data(13) + CRC(4)
    return header;
}

std::vector<unsigned char> png::getPLTE(const std::vector<unsigned char> &binary)
{
}
