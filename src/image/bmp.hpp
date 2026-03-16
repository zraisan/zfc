#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace bmp {
struct FileHeader {
  char fileType[2];
  uint32_t fileSize;
  uint32_t offset;
  int32_t width;
  int32_t height;
  uint16_t bitsPerPixel;
};

FileHeader readHeader(const std::vector<unsigned char> &binary);
std::vector<unsigned char>
readImageData(std::vector<unsigned char> &imageBinary);

template <typename FH>
void generate(FH &imageHeader, std::vector<unsigned char> &imageData,
              std::string outputPath) {

  std::ofstream output(outputPath, std::ios::binary);
  uint8_t zero8 = 0;
  uint16_t zero16 = 0;
  uint32_t zero32 = 0;
  uint32_t dibSize = 40;
  uint16_t colorPlanes = 1;
  uint32_t offset = 14 + dibSize;
  uint32_t compressionMethod = 0;
  uint32_t imageSize = static_cast<uint32_t>(imageData.size());
  int32_t resolution = 2835; // 72 DPI in pixels per metre

  /* BMP File Header */
  output.write(
      "BM",
      2); // 0: (2B) Header field - 0x42 0x4D (BM) for Windows 3.1x, 95, NT, ...
  output.write(reinterpret_cast<const char *>(&imageHeader.fileSize),
               4); // 2: (4B) The size of the BMP file in bytes
  output.write(reinterpret_cast<const char *>(&zero16),
               2); // 6: (2B) Reserved; if created manually can be 0
  output.write(reinterpret_cast<const char *>(&zero16),
               2); // 8: (2B) Reserved; if created manually can be 0
  output.write(
      reinterpret_cast<const char *>(&offset),
      4); // 10: (4B) The offset of the byte where the pixel array can be found

  /* DIB Header */
  output.write(reinterpret_cast<const char *>(&dibSize),
               4); // 14: (4B) The size of this header, in bytes (40)
  output.write(reinterpret_cast<const char *>(&imageHeader.width),
               4); // 18: (4B) The bitmap width in pixels (signed integer)
  output.write(reinterpret_cast<const char *>(&imageHeader.height),
               4); // 22: (4B) The bitmap height in pixels (signed integer)
  output.write(reinterpret_cast<const char *>(&colorPlanes),
               2); // 26: (2B) The number of color planes (must be 1)
  output.write(
      reinterpret_cast<const char *>(&imageHeader.bitsPerPixel),
      2); // 28: (2B) The number of bits per pixel (1, 4, 8, 16, 24, 32)
  output.write(reinterpret_cast<const char *>(&compressionMethod),
               4); // 30: (4B) The compression method being used
  output.write(
      reinterpret_cast<const char *>(&imageSize),
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
  int paddedRow = (imageHeader.width * 3 + 3) & ~3;
  for (int y = imageHeader.height - 1; y >= 0; y--) {
    for (int x = 0; x < imageHeader.width * 3; x += 3) {
      int i = y * imageHeader.width * 3 + x;
      uint8_t b = imageData[i + 2];
      uint8_t g = imageData[i + 1];
      uint8_t r = imageData[i];
      output.write(reinterpret_cast<const char *>(&b), 1);
      output.write(reinterpret_cast<const char *>(&g), 1);
      output.write(reinterpret_cast<const char *>(&r), 1);
    }
    for (int p = imageHeader.width * 3; p < paddedRow; p++) {
      output.write(reinterpret_cast<const char *>(&zero8), 1);
    }
  }
}
} // namespace bmp
