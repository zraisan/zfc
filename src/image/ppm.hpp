#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace ppm {
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
void encode(FH &image_header, std::vector<unsigned char> &image_binary,
              std::string output_path) {
  std::ofstream output(output_path, std::ios::binary);
  output << "P6\n"
         << image_header.width << " " << image_header.height << "\n255\n";

  int ch = image_header.channels;
  for (int y = 0; y < image_header.height; y++) {
    for (int x = 0; x < image_header.width; x++) {
      int i = (y * image_header.width + x) * ch;
      output.put(image_binary[i]);
      output.put(image_binary[i + 1]);
      output.put(image_binary[i + 2]);
      // PPM P6 has no alpha support, always output 3 channels
    }
  }
}
} // namespace ppm
