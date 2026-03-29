#include "tga.hpp"
#include <vector>

tga::FileHeader tga::readHeader(std::vector<unsigned char> &binary)
{
    tga::FileHeader header;
    header.fileIdLength = binary[0];
    header.width = binary[12] | (binary[13] << 8);
    header.height = binary[14] | (binary[15] << 8);
    header.bitsPerPixel = binary[16];
    header.rightToLeft = (binary[17] >> 4) & 1;
    header.topToBottom = (binary[17] >> 5) & 1;
    header.offset = 18 + header.fileIdLength;
    header.channels = header.bitsPerPixel / 8;
    return header;
}

std::vector<unsigned char>
tga::readImageData(std::vector<unsigned char> &imageBinary)
{
    tga::FileHeader header = tga::readHeader(imageBinary);
    int y;
    bool condition;
    int increment;
    std::vector<unsigned char> imageData;
    for (int y = 0; y < header.height; y++)
    {
        int row = header.topToBottom ? y : header.height - 1 - y;
        for (int x = 0; x < header.width * 3; x += 3)
        {
            int col = header.rightToLeft ? x : header.width * 3 - 1 - x;
            int r = col + 2;
            int g = col + 1;
            int b = col;
            imageData.push_back(imageBinary[header.offset + r]);
            imageData.push_back(imageBinary[header.offset + g]);
            imageData.push_back(imageBinary[header.offset + b]);
        }
    }
    return imageData;
}