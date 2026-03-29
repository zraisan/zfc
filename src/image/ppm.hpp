#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace ppm {
struct FileHeader {
  char fileType[2];
  uint32_t fileSize;
  uint32_t offset;
  int32_t width;
  int32_t height;
  uint16_t bitsPerPixel;
  uint8_t channels;
};

FileHeader readHeader(const std::vector<unsigned char> &binary);
std::vector<unsigned char>
readImageData(std::vector<unsigned char> &imageBinary);

template <typename FH>
void generate(FH &imageHeader, std::vector<unsigned char> &imageBinary,
              std::string outputPath) {
  std::ofstream output(outputPath, std::ios::binary);
  output << "P6\n"
         << imageHeader.width << " " << imageHeader.height << "\n255\n";

  int ch = imageHeader.channels;
  for (int y = 0; y < imageHeader.height; y++) {
    for (int x = 0; x < imageHeader.width; x++) {
      int i = (y * imageHeader.width + x) * ch;
      output.put(imageBinary[i]);
      output.put(imageBinary[i + 1]);
      output.put(imageBinary[i + 2]);
      // PPM P6 has no alpha support, always output 3 channels
    }
  }
}
} // namespace ppm
