#pragma once

#include "png_filter.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <math.h>
#include <vector>
#include <numeric>

namespace png
{
    // PNG CRC32 (polynomial 0xEDB88320)
    inline uint32_t crc32(const unsigned char *data, size_t len)
    {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < len; i++)
        {
            crc ^= data[i];
            for (int j = 0; j < 8; j++)
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
        return crc ^ 0xFFFFFFFF;
    }

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

        auto write_be32 = [&output](uint32_t v)
        {
            output.put((v >> 24) & 0xff);
            output.put((v >> 16) & 0xff);
            output.put((v >> 8) & 0xff);
            output.put(v & 0xff);
        };

        auto push_be32 = [](std::vector<unsigned char> &buf, uint32_t v)
        {
            buf.push_back((v >> 24) & 0xff);
            buf.push_back((v >> 16) & 0xff);
            buf.push_back((v >> 8) & 0xff);
            buf.push_back(v & 0xff);
        };

        // Writes length + type + data + CRC (CRC covers type + data)
        auto write_chunk = [&output, &write_be32](const char *type,
                                                  const std::vector<unsigned char> &data)
        {
            write_be32(data.size());
            output.write(type, 4);
            output.write(reinterpret_cast<const char *>(data.data()), data.size());
            std::vector<unsigned char> crc_input(type, type + 4);
            crc_input.insert(crc_input.end(), data.begin(), data.end());
            write_be32(png::crc32(crc_input.data(), crc_input.size()));
        };

        // PNG signature
        output.write("\x89PNG\r\n\x1a\n", 8);

        // IHDR
        std::vector<unsigned char> ihdr;
        push_be32(ihdr, image_header.width);
        push_be32(ihdr, image_header.height);
        ihdr.push_back(image_header.bits_per_pixel / image_header.channels); // bit depth
        switch (image_header.channels)
        {
        case 1:
            ihdr.push_back(0); // greyscale
            break;
        case 2:
            ihdr.push_back(4); // greyscale + alpha
            break;
        case 3:
            ihdr.push_back(2); // truecolor
            break;
        case 4:
            ihdr.push_back(6); // truecolor + alpha
            break;
        }
        ihdr.push_back(0); // compression method
        ihdr.push_back(0); // filter method
        ihdr.push_back(0); // interlace method
        write_chunk("\x49\x48\x44\x52", ihdr);

        // PLTE (only valid for truecolor types 2 and 6)
        if (image_header.channels == 3 || image_header.channels == 4)
        {
            std::vector<unsigned char> plte;
            for (int i = 0; i < 256; i++)
            {
                plte.push_back(i); // R
                plte.push_back(i); // G
                plte.push_back(i); // B
            }
            write_chunk("\x50\x4C\x54\x45", plte);
        }

        // IDAT
        std::vector<unsigned char> filtered_data =
            msad_heuristic(image_header, image_data);
        std::vector<unsigned char> compressed_data = deflate::deflate(filtered_data);
        std::vector<unsigned char> zlib_data = deflate::make_zlib(filtered_data, compressed_data);
        write_chunk("\x49\x44\x41\x54", zlib_data);

        // IEND (empty data)
        write_chunk("\x49\x45\x4E\x44", {});
    }

    template <typename FH>
    std::vector<unsigned char> msad_heuristic(const FH &header,
                                              std::vector<unsigned char> &row_binary)
    {

        std::vector<unsigned char> output;
        for (int i = 0; i < header.height; i += 1)
        {
            std::vector<unsigned char> filter_0(row_binary.begin() + i * header.width * header.channels,
                                                row_binary.begin() + (i + 1) * header.width * header.channels);
            std::vector<unsigned char> filter_1(row_binary.begin() + i * header.width * header.channels,
                                                row_binary.begin() + (i + 1) * header.width * header.channels);
            std::vector<unsigned char> filter_2(row_binary.begin() + i * header.width * header.channels,
                                                row_binary.begin() + (i + 1) * header.width * header.channels);
            std::vector<unsigned char> filter_3(row_binary.begin() + i * header.width * header.channels,
                                                row_binary.begin() + (i + 1) * header.width * header.channels);
            std::vector<unsigned char> filter_4(row_binary.begin() + i * header.width * header.channels,
                                                row_binary.begin() + (i + 1) * header.width * header.channels);
            for (int j = 0; j < header.width * header.channels; j += header.channels)
            {
                uint8_t a = (j >= header.channels) ? row_binary[i * header.width * header.channels + j - header.channels]
                                                   : 0; // Previous pixel on the same line
                uint8_t b = (i >= 1)
                                ? row_binary[(i - 1) * header.width * header.channels + j]
                                : 0; // Pixel on the previous line
                uint8_t c =
                    (j >= header.channels && i >= 1)
                        ? row_binary[(i - 1) * header.width * header.channels + j - header.channels]
                        : 0; // Previous pixel on the previous line

                filter_1[j] = filter::filter_sub(filter_1[j], a);
                filter_2[j] = filter::filter_up(filter_2[j], b);
                filter_3[j] = filter::filter_average(filter_3[j], a, b);
                filter_4[j] = filter::filter_paeth(filter_4[j], a, b, c);
            }
            auto msad = [](long long s, unsigned char b)
            { return s + std::abs((int8_t)b); }; // Sum function that returns signed integer absolute difference
            long long filter_0_sum = std::accumulate(filter_0.begin(), filter_0.end(), 0LL, msad);
            long long filter_1_sum = std::accumulate(filter_1.begin(), filter_1.end(), 0LL, msad);
            long long filter_2_sum = std::accumulate(filter_2.begin(), filter_2.end(), 0LL, msad);
            long long filter_3_sum = std::accumulate(filter_3.begin(), filter_3.end(), 0LL, msad);
            long long filter_4_sum = std::accumulate(filter_4.begin(), filter_4.end(), 0LL, msad);
            long long min_sum = std::min({filter_0_sum, filter_1_sum, filter_2_sum, filter_3_sum, filter_4_sum});
            if (min_sum == filter_0_sum)
            {
                output.push_back(0);
                output.insert(output.end(), filter_0.begin(), filter_0.end());
            }
            else if (min_sum == filter_1_sum)
            {
                output.push_back(1);
                output.insert(output.end(), filter_1.begin(), filter_1.end());
            }
            else if (min_sum == filter_2_sum)
            {
                output.push_back(2);
                output.insert(output.end(), filter_2.begin(), filter_2.end());
            }
            else if (min_sum == filter_3_sum)
            {
                output.push_back(3);
                output.insert(output.end(), filter_3.begin(), filter_3.end());
            }
            else if (min_sum == filter_4_sum)
            {
                output.push_back(4);
                output.insert(output.end(), filter_4.begin(), filter_4.end());
            }
        }
        return output;
    }

    FileHeader read_header(const std::vector<unsigned char> &binary);
    std::vector<png::RGB> read_plte(const std::vector<unsigned char> &binary,
                                    uint8_t channels, int length);
    std::vector<unsigned char> read_idat(const png::FileHeader &header,
                                         const std::vector<unsigned char> &binary,
                                         const std::vector<png::RGB> &palette);
    std::vector<unsigned char>
    decode(const std::vector<unsigned char> &image_binary);
} // namespace png
