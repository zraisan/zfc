#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

namespace tga
{
    struct FileHeader
    {
        uint8_t fileIdLength;
        uint32_t offset;
        uint16_t width;
        uint16_t height;
        bool rightToLeft;
        bool topToBottom;
        uint8_t bitsPerPixel;
        uint8_t channels;
    };

    FileHeader readHeader(std::vector<unsigned char> &binary);
    std::vector<unsigned char>
    readImageData(std::vector<unsigned char> &imageBinary);

    template <typename FH>
    void generate(FH &imageHeader, std::vector<unsigned char> &imageBinary,
                  std::string outputPath)
    {
        std::ofstream output(outputPath, std::ios::binary);
        uint8_t zero8 = 0;
        uint16_t zero16 = 0;
        int zero = 0;
        u_int8_t imageDescriptor = 32;

        output.write(reinterpret_cast<const char *> & zero8, 1);
        output.write(reinterpret_cast<const char *> & zero8, 1);
        output.write(reinterpret_cast<const char *> & zero8, 1);
        output.write(reinterpret_cast<const char *> & zero, 5);
        output.write(reinterpret_cast<const char *> & zero16, 2);
        output.write(reinterpret_cast<const char *> & zero16, 2);
        output.write(reinterpret_cast<const char *> & imageHeader.width, 2);
        output.write(reinterpret_cast<const char *> & imageHeader.height, 2);
        output.write(reinterpret_cast<const char *> & imageHeader.bitsPerPixel, 1);
        output.write(reinterpret_cast<const char *> & imageDescriptor, 1);
    }

} // namespace tga