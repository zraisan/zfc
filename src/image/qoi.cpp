#include "qoi.hpp"

enum class EncodingMethod : uint8_t
{
    QOI_OP_RGB = 0,
    QOI_OP_RGBA = 1,
    QOI_OP_INDEX = 2,
    QOI_OP_DIFF = 3,
    QOI_OP_LUMA = 4,
    QOI_OP_RUN = 5
};

qoi::FileHeader qoi::readHeader(const std::vector<unsigned char> &binary)
{
    qoi::FileHeader header;
    header.width = binary[4] | binary[5] | binary[6] | binary[7];
    header.height = binary[8] | binary[9] | binary[10] | binary[11];
    header.channels = binary[12];
    header.colorspace = binary[13];
    header.offset = 14;
    return header;
}

std::vector<unsigned char>
qoi::readImageData(std::vector<unsigned char> &imageBinary)
{
    qoi::FileHeader header = qoi::readHeader(imageBinary);
    EncodingMethod emethod;
    uint8_t firstByte = imageBinary[header.offset];
    if (firstByte == 254) // 11111110
    {
        emethod = EncodingMethod::QOI_OP_RGB;
    }
    else if (firstByte == 255) // 11111111
    {
        emethod = EncodingMethod::QOI_OP_RGBA;
    }
    else if (firstByte <= 63) // 00...
    {
        emethod = EncodingMethod::QOI_OP_INDEX;
    }
    else if (firstByte <= 127 and firstByte >= 64) // 01...
    {
        emethod = EncodingMethod::QOI_OP_DIFF;
    }
    else if (firstByte <= 191 and firstByte >= 128) // 10...
    {
        emethod = EncodingMethod::QOI_OP_LUMA;
    }
    else // 11...
    {
        emethod = EncodingMethod::QOI_OP_RUN;
    }
}
