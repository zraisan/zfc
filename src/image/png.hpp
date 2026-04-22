#pragma once

#include "png_filter.hpp"
#include <algorithm>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <math>
#include <vector>

namespace png {
struct RGB {
  uint8_t r, g, b;
};

struct FileHeader {
  uint32_t width;
  uint32_t height;
  uint8_t channels;
  uint16_t bits_per_pixel;
  uint32_t offset;
};

template <typename FH>
void encode(FH &image_header, std::vector<unsigned char> &image_data,
            std::string output_path) {
  std::ofstream output(output_path, std::ios::binary);

  // PNG signature
  output.write("\x89PNG\r\n\x1a\n", 8);

  // IHDR chunk
  output.write("\x49\x48\x44\x52", 4);
  output.write(reinterpret_cast<const char *>(&image_header.width), 4);
  output.write(reinterpret_cast<const char *>(&image_header.height), 4);
  output.put(image_header.bits_per_pixel / image_header.channels); // bit depth
  switch (image_header.channels) {
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

  // PLTE chunk
  output.write(reinterpret_cast<const char *>("\x50\x4C\x54\x45"),
               4); // PLTE signature
  std::bitset<24> palette[256];
  for (int i = 0; i < 256; i++) {
    palette[i] = i;
  }

  // IDAT
  output.write(reinterpret_cast<const char *>("\x49\x44\x41\x54"), 4);
  for (int i = 0; i < image_data.size(); i++) {
  }
}

template <typename FH>
uint8_t msad_heuristic(const FH &header,
                       std::vector<unsigned char> row_binary = [0]) {
  unsigned char filter_1[row_binary.size()] = row_binary;
  unsigned char filter_2[row_binary.size()] = row_binary;
  unsigned char filter_3[row_binary.size()] = row_binary;
  unsigned char filter_4[row_binary.size()] = row_binary;
  std::vector<unsigned char> output;
  for (int i = 0; i < header.height; i++) {
    for (int j = 0; j < header.width * header.channels; i += header.channels) {
      uint8_t a = (i >= header.channels) ? filter_1[i - header.channels]
                                         : 0; // Previous pixel on the same line
      uint8_t b = (i >= header.width * header.channels)
                      ? filter_1[i - header.width * header.channels]
                      : 0; // Pixel on the previous line
      uint8_t c =
          (i >= header.channels && i >= header.width * header.channels)
              ? filter_1[i - header.width * header.channels - header.channels]
              : 0; // Previous pixel on the previous line

      filter_1[i] = filter::filter_sub(filter_1[i], a);
      filter_2[i] = filter::filter_up(filter_2[i], b);
      filter_3[i] = filter::filter_average(filter_3[i], a, b);
      filter_4[i] = filter::filter_paeth(filter_4[i], a, b, c);
    }
  }
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
