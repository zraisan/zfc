#pragma once

#include <cstdint>
#include <vector>

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
