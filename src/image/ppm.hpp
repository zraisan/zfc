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

  for (int y = 0; y < imageHeader.height; y++) {
    for (int x = 0; x < imageHeader.width * 3; x++) {
      int i = y * imageHeader.width * 3 + x;
      output.put(imageBinary[i]);
    }
  }
}
} // namespace ppm
