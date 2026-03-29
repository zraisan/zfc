#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace qoi {
struct FileHeader {
  uint32_t width;
  uint32_t height;
  uint8_t channels;
  uint8_t colorspace;
  uint32_t offset;
};

FileHeader readHeader(const std::vector<unsigned char> &binary);
std::vector<unsigned char>
readImageData(std::vector<unsigned char> &imageBinary);

template <typename FH>
void generate(FH &imageHeader, std::vector<unsigned char> &imageBinary,
              std::string outputPath) {
  std::ofstream output(outputPath, std::ios::binary);
  uint32_t width = imageHeader.width;
  uint32_t height = imageHeader.height;

  output.write("qoif", 4);
  output.write(reinterpret_cast<const char *>(&width), 4);
  output.write(reinterpret_cast<const char *>(&height), 4);

  for (int y = 0; y < imageHeader.height; y++) {
    for (int x = 0; x < imageHeader.width * 3; x++) {
      int i = y * imageHeader.width * 3 + x;
      output.put(imageBinary[i]);
    }
  }
}

} // namespace qoi
