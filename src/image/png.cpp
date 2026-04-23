#include "png.hpp"
#include "deflate.hpp"
#include "png_filter.hpp"
#include <cassert>
#include <string>

png::FileHeader png::read_header(const std::vector<unsigned char> &binary)
{
    png::FileHeader header;
    header.width =
        (binary[16] << 24) | (binary[17] << 16) | (binary[18] << 8) | binary[19];
    header.height =
        (binary[20] << 24) | (binary[21] << 16) | (binary[22] << 8) | binary[23];

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
    header.offset =
        33; // signature(4) + length(4) + IHDR(4) + IHDR data(13) + CRC(4)
    return header;
}

std::vector<unsigned char>
png::decode(const std::vector<unsigned char> &image_binary)
{
    png::FileHeader header = read_header(image_binary);
    std::vector<png::RGB> palette;
    std::vector<unsigned char> decoded_image;

    int idx = header.offset;
    while (idx < image_binary.size())
    {
        std::string type(image_binary.begin() + idx + 4,
                         image_binary.begin() + idx + 8);
        int length = (image_binary[idx] << 24) | (image_binary[idx + 1] << 16) |
                     (image_binary[idx + 2] << 8) | image_binary[idx + 3];

        if (type == "PLTE")
            palette =
                png::read_plte(std::vector(image_binary.begin() + idx + 8,
                                           image_binary.begin() + idx + 8 + length),
                               header.channels, length);
        if (type == "IDAT")
            decoded_image =
                png::read_idat(header,
                               std::vector(image_binary.begin() + idx + 8,
                                           image_binary.begin() + idx + 8 + length),
                               palette);

        idx += length + 12; // traverse by data length (Chunk data) and overhead
                            // length (Length + Chunk type + CRC)
    }
    return decoded_image;
}

std::vector<png::RGB> png::read_plte(const std::vector<unsigned char> &binary,
                                     uint8_t channels, int length)
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

std::vector<unsigned char>
png::read_idat(const png::FileHeader &header,
               const std::vector<unsigned char> &binary,
               const std::vector<png::RGB> &palette)
{
    std::vector<unsigned char> idat_binary = deflate::inflate(binary);
    std::vector<unsigned char> defiltered_binary;
    std::vector<unsigned char> decoded_binary;

    for (int i = 0; i < idat_binary.size();)
    {
        uint8_t filter_type = idat_binary[i];
        i++;
        for (int j = 0; j < header.width * header.channels; j++)
        {
            uint8_t x = idat_binary[i + j];
            uint8_t a =
                (j >= header.channels)
                    ? defiltered_binary[defiltered_binary.size() - header.channels]
                    : 0; // Previous pixel on the same line
            uint8_t b = (defiltered_binary.size() >= header.width * header.channels)
                            ? defiltered_binary[defiltered_binary.size() -
                                                header.width * header.channels]
                            : 0; // Pixel on the previous line
            uint8_t c = (j >= header.channels &&
                         defiltered_binary.size() >= header.width * header.channels)
                            ? defiltered_binary[defiltered_binary.size() -
                                                header.width * header.channels -
                                                header.channels]
                            : 0; // Previous pixel on the previous line

            switch (filter_type)
            {
            case 0:
                defiltered_binary.push_back(x);
                break;
            case 1:
                defiltered_binary.push_back(filter::defilter_sub(x, a));
                break;
            case 2:
                defiltered_binary.push_back(filter::defilter_up(x, b));
                break;
            case 3:
                defiltered_binary.push_back(filter::defilter_average(x, a, b));
                break;
            case 4:
                defiltered_binary.push_back(filter::defilter_paeth(x, a, b, c));
                break;
            }
        }
        i += header.width * header.channels;
    }

    if (!palette.empty())
    {
        for (int i = 0; i < defiltered_binary.size(); i++)
        {
            uint8_t index = defiltered_binary[i];
            png::RGB color = palette[index];
            decoded_binary.push_back(color.r);
            decoded_binary.push_back(color.g);
            decoded_binary.push_back(color.b);
        }
        return decoded_binary;
    }
    else
    {
        decoded_binary = defiltered_binary;
    }

    return decoded_binary;
}
