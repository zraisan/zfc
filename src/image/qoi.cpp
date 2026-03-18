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

EncodingMethod getEncodingMethod(uint8_t byte)
{
    if (byte == 254)
        return EncodingMethod::QOI_OP_RGB;
    else if (byte == 255)
        return EncodingMethod::QOI_OP_RGBA;
    else if ((byte >> 6) == 0)
        return EncodingMethod::QOI_OP_INDEX;
    else if ((byte >> 6) == 1)
        return EncodingMethod::QOI_OP_DIFF;
    else if ((byte >> 6) == 3)
        return EncodingMethod::QOI_OP_RUN;
    else
        return EncodingMethod::QOI_OP_LUMA;
}

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
    std::vector<unsigned char> imageData;

    int idx = header.offset;
    while (idx < imageBinary.size() - header.offset)
    {
        EncodingMethod emethod = getEncodingMethod(imageBinary[idx]);
        switch (emethod)
        {
        case EncodingMethod::QOI_OP_RGB:
            break;
        case EncodingMethod::QOI_OP_RGBA:
            break;
        case EncodingMethod::QOI_OP_INDEX:
            break;
        case EncodingMethod::QOI_OP_DIFF:
            break;
        case EncodingMethod::QOI_OP_LUMA:
            break;
        case EncodingMethod::QOI_OP_RUN:
            break;
        }
    }
}
