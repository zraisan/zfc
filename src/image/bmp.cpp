#include "bmp.hpp"
#include <cstdint>
#include <vector>

bmp::FileHeader bmp::read_header(const std::vector<unsigned char> &binary)
{
  uint32_t size =
      binary[2] | (binary[3] << 8) | (binary[4] << 16) | (binary[5] << 24);

  uint32_t offset =
      binary[10] | (binary[11] << 8) | (binary[12] << 16) | (binary[13] << 24);

  int32_t width =
      binary[18] | (binary[19] << 8) | (binary[20] << 16) | (binary[21] << 24);
  int32_t height =
      binary[22] | (binary[23] << 8) | (binary[24] << 16) | (binary[25] << 24);
  uint16_t bits_per_pixel = binary[28] | (binary[29] << 8);

  return {{static_cast<char>(binary[0]), static_cast<char>(binary[1])},
          size,
          offset,
          width,
          height,
          bits_per_pixel,
          static_cast<uint8_t>(bits_per_pixel / 8)};
}

std::vector<unsigned char>
bmp::decode(std::vector<unsigned char> &image_binary)
{
  bmp::FileHeader header = bmp::read_header(image_binary);

  std::vector<unsigned char> unpadded_binary(header.height * header.width * 3);

  int padded_row = (header.width * 3 + 3) & ~3;
  for (int y = header.height - 1; y >= 0; y--)
  {
    for (int x = 0; x < header.width * 3; x += 3)
    {
      int dist = y * header.width * 3 + x;
      int src = (header.height - 1 - y) * padded_row + x;
      unpadded_binary[dist + 2] = image_binary[header.offset + src];     // R
      unpadded_binary[dist + 1] = image_binary[header.offset + src + 1]; // G
      unpadded_binary[dist] = image_binary[header.offset + src + 2];     // B
    }
  }

  return unpadded_binary;
}
