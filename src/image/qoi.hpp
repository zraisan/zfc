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

  int ch = imageHeader.channels;
  for (int y = 0; y < imageHeader.height; y++) {
    for (int x = 0; x < imageHeader.width; x++) {
      int i = (y * imageHeader.width + x) * ch;
      output.put(imageBinary[i]);
      output.put(imageBinary[i + 1]);
      output.put(imageBinary[i + 2]);
      if (ch == 4)
        output.put(imageBinary[i + 3]);
    }
  }
}

} // namespace qoi
