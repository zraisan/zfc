#include "png.hpp"
#include <string>
#include <cassert>

png::FileHeader png::read_header(const std::vector<unsigned char> &binary)
{
    png::FileHeader header;
    header.width = (binary[16] << 24) | (binary[17] << 16) | (binary[18] << 8) | binary[19];
    header.height = (binary[20] << 24) | (binary[21] << 16) | (binary[22] << 8) | binary[23];

    uint8_t bit_depth = binary[24];
    uint8_t color_type = binary[25];

    switch (color_type)
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

    header.bits_per_pixel = bit_depth * header.channels;
    header.offset = 33; // signature(4) + length(4) + IHDR(4) + IHDR data(13) + CRC(4)
    return header;
}

std::vector<unsigned char> png::decode(std::vector<unsigned char> &image_binary)
{
    png::FileHeader header = read_header(image_binary);
    std::vector<png::RGB> palette;

    int idx = header.offset;
    while (idx < image_binary.size())
    {
        std::string type(image_binary.begin() + idx + 4, image_binary.begin() + idx + 8);
        int length = (image_binary[idx] << 24) | (image_binary[idx + 1] << 16) | (image_binary[idx + 2] << 8) | image_binary[idx + 3];

        if (type == "PLTE")
            palette = png::read_plte(std::vector(image_binary.begin() + idx + 8, image_binary.begin() + idx + 8 + length), header.channels, length);
        if (type == "IDAT")
            png::read_idat(std::vector(image_binary.begin() + idx + 8, image_binary.begin() + idx + 8 + length), palette);
        if (type == "IEND")
            png::read_iend(std::vector(image_binary.begin() + idx + 8, image_binary.end()));

        idx += length + 12; // traverse by data length (Chunk data) and overhead length (Length + Chunk type + CRC)
    }
}

std::vector<png::RGB> png::read_plte(const std::vector<unsigned char> &binary, uint8_t channels, int length)
{
    int palette_number = length / 3;
    assert(length % 3 != 0 || palette_number <= 0 || palette_number > 256);
    assert(channels == 1 || channels == 2);
    std::vector<png::RGB> plte_binary(256);
    for (int i = 0; i < palette_number; i++)
    {
        plte_binary[i] = {binary[i * 3], binary[i * 3 + 1], binary[i * 3 + 2]};
    }
    for (int i = palette_number; i < 256; i++)
    {
        plte_binary[i] = {0, 0, 0}; // Fill remaining palette entries with black
    }
    return plte_binary;
}

std::vector<unsigned char> png::read_idat(const std::vector<unsigned char> &binary, const std::vector<png::RGB> &palette)
{
}

std::vector<unsigned char> png::read_iend(const std::vector<unsigned char> &binary)
{
}
