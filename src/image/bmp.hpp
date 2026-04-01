#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace bmp {
struct FileHeader {
  char file_type[2];
  uint32_t file_size;
  uint32_t offset;
  int32_t width;
  int32_t height;
  uint16_t bits_per_pixel;
  uint8_t channels;
};

FileHeader read_header(const std::vector<unsigned char> &binary);
std::vector<unsigned char>
decode(std::vector<unsigned char> &image_binary);

template <typename FH>
void encode(FH &image_header, std::vector<unsigned char> &image_data,
              std::string output_path) {

  std::ofstream output(output_path, std::ios::binary);
  uint8_t zero8 = 0;
  uint16_t zero16 = 0;
  uint32_t zero32 = 0;
  uint32_t dib_size = 40;
  uint16_t color_planes = 1;
  uint32_t offset = 14 + dib_size;
  uint32_t compression_method = 0;
  uint32_t image_size = static_cast<uint32_t>(image_data.size());
  int32_t resolution = 2835; // 72 DPI in pixels per metre

  /* BMP File Header */
  output.write(
      "BM",
      2); // 0: (2B) Header field - 0x42 0x4D (BM) for Windows 3.1x, 95, NT, ...
  output.write(reinterpret_cast<const char *>(&image_header.file_size),
               4); // 2: (4B) The size of the BMP file in bytes
  output.write(reinterpret_cast<const char *>(&zero16),
               2); // 6: (2B) Reserved; if created manually can be 0
  output.write(reinterpret_cast<const char *>(&zero16),
               2); // 8: (2B) Reserved; if created manually can be 0
  output.write(
      reinterpret_cast<const char *>(&offset),
      4); // 10: (4B) The offset of the byte where the pixel array can be found

  /* DIB Header */
  output.write(reinterpret_cast<const char *>(&dib_size),
               4); // 14: (4B) The size of this header, in bytes (40)
  output.write(reinterpret_cast<const char *>(&image_header.width),
               4); // 18: (4B) The bitmap width in pixels (signed integer)
  output.write(reinterpret_cast<const char *>(&image_header.height),
               4); // 22: (4B) The bitmap height in pixels (signed integer)
  output.write(reinterpret_cast<const char *>(&color_planes),
               2); // 26: (2B) The number of color planes (must be 1)
  output.write(
      reinterpret_cast<const char *>(&image_header.bits_per_pixel),
      2); // 28: (2B) The number of bits per pixel (1, 4, 8, 16, 24, 32)
  output.write(reinterpret_cast<const char *>(&compression_method),
               4); // 30: (4B) The compression method being used
  output.write(
      reinterpret_cast<const char *>(&image_size),
      4); // 34: (4B) The image size (raw bitmap data size, 0 for BI_RGB)
  output.write(
      reinterpret_cast<const char *>(&resolution),
      4); // 38: (4B) Horizontal resolution (pixel per metre, signed integer)
  output.write(
      reinterpret_cast<const char *>(&resolution),
      4); // 42: (4B) Vertical resolution (pixel per metre, signed integer)
  output.write(
      reinterpret_cast<const char *>(&zero32),
      4); // 46: (4B) Number of colors in palette, or 0 to default to 2^n
  output.write(
      reinterpret_cast<const char *>(&zero32),
      4); // 50: (4B) Number of important colors, or 0 when all are important

  /* Image Data */
  int ch = image_header.channels;
  int padded_row = ch == 4 ? image_header.width * 4 : (image_header.width * 3 + 3) & ~3;
  for (int y = image_header.height - 1; y >= 0; y--) {
    for (int x = 0; x < image_header.width * ch; x += ch) {
      int i = y * image_header.width * ch + x;
      output.write(reinterpret_cast<const char *>(&image_data[i + 2]), 1); // B
      output.write(reinterpret_cast<const char *>(&image_data[i + 1]), 1); // G
      output.write(reinterpret_cast<const char *>(&image_data[i]),     1); // R
      if (ch == 4)
        output.write(reinterpret_cast<const char *>(&image_data[i + 3]), 1); // A
    }
    if (ch == 3) {
      for (int p = image_header.width * 3; p < padded_row; p++)
        output.write(reinterpret_cast<const char *>(&zero8), 1);
    }
  }
}
} // namespace bmp
