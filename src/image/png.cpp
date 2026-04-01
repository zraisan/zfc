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

std::vector<unsigned char> png::readImageData(std::vector<unsigned char> &imageBinary)
{
    png::FileHeader header = readHeader(imageBinary);

    int idx = header.offset;
    while (idx < imageBinary.size())
    {
        std::string type(imageBinary.begin() + idx + 4, imageBinary.begin() + idx + 8);

        if (type == "PLTE")
            png::readPLTE(std::vector(imageBinary.begin() + idx + 8, imageBinary.end()));
        if (type == "IDAT")
            png::readIDAT(std::vector(imageBinary.begin() + idx + 8, imageBinary.end()));
        if (type == "IEND")
            png::readIEND(std::vector(imageBinary.begin() + idx + 8, imageBinary.end()));

        idx += (imageBinary[idx] << 24) | (imageBinary[idx + 1] << 16) | (imageBinary[idx + 2] << 8) | imageBinary[idx + 3] + 12; // traverse by data length (Chunk data) and overhead length (Length + Chunk type + CRC)
    }
}

std::vector<unsigned char> png::readPLTE(const std::vector<unsigned char> &binary)
{
}

std::vector<unsigned char> png::readIDAT(const std::vector<unsigned char> &binary)
{
}

std::vector<unsigned char> png::readIEND(const std::vector<unsigned char> &binary)
{
}
