#pragma once

#include <cstdint>
#include <vector>
#include <fstream>

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

    template <typename FH>
    void encode(FH &image_header, std::vector<unsigned char> &image_data,
                std::string output_path)
    {
        std::ofstream output(output_path, std::ios::binary);

        // PNG signature
        output.write("\x89PNG\r\n\x1a\n", 8);

        // IHDR chunk
        output.write("\x49\x48\x44\x52", 4);
        output.write(reinterpret_cast<const char *>(&image_header.width), 4);
        output.write(reinterpret_cast<const char *>(&image_header.height), 4);
        output.put(image_header.bits_per_pixel / image_header.channels); // bit depth
        switch (image_header.channels)
        {
        case 1:
            output.put(0); // color type: greyscale
            break;
        case 2:
            output.put(4); // color type: greyscale + alpha
            break;
        case 3:
            output.put(2); // color type: truecolor
            break;
        case 4:
            output.put(6); // color type: truecolor + alpha
            break;
        }
        output.put(0); // compression method
        output.put(0); // filter method
        output.put(0); // interlace method. TODO: implemented Adam7 interlace
    }

    FileHeader read_header(const std::vector<unsigned char> &binary);
    std::vector<png::RGB> read_plte(const std::vector<unsigned char> &binary, uint8_t channels, int length);
    std::vector<unsigned char> read_idat(const png::FileHeader &header, const std::vector<unsigned char> &binary, const std::vector<png::RGB> &palette);
    std::vector<unsigned char>
    decode(const std::vector<unsigned char> &image_binary);
}