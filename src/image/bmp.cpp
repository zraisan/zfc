#include "bmp.hpp"
#include <cstdint>
#include <vector>

FileHeader readHeader(const std::vector<unsigned char> &binary) {
  uint32_t size =
      binary[2] | (binary[3] << 8) | (binary[4] << 16) | (binary[5] << 24);

  uint32_t offset =
      binary[10] | (binary[11] << 8) | (binary[12] << 16) | (binary[13] << 24);

  int32_t width =
      binary[18] | (binary[19] << 8) | (binary[20] << 16) | (binary[21] << 24);
  int32_t height =
      binary[22] | (binary[23] << 8) | (binary[24] << 16) | (binary[25] << 24);
  uint16_t bitsPerPixel = binary[28] | (binary[29] << 8);

  return {{static_cast<char>(binary[0]), static_cast<char>(binary[1])},
          size,
          offset,
          width,
          height,
          bitsPerPixel};
}

std::vector<unsigned char>
readImageData(std::vector<unsigned char> &imageBinary) {
  FileHeader header = readHeader(imageBinary);

  std::vector<unsigned char> unpaddedBinary(header.height * header.width * 3);

  int paddedRow = (header.width * 3 + 3) & ~3;
  for (int y = header.height - 1; y >= 0; y--) {
    for (int x = 0; x < header.width * 3; x++) {
      int dist = y * header.width * 3 + x;
      int src = y * paddedRow + x;
      unpaddedBinary[dist] = imageBinary[header.offset + src];
    }
  }

  return unpaddedBinary;
}
