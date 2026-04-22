#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace tga {
struct FileHeader {
  uint8_t file_id_length;
  uint32_t offset;
  uint16_t width;
  uint16_t height;
  bool right_to_left;
  bool top_to_bottom;
  uint8_t bits_per_pixel;
  uint8_t channels;
};

FileHeader read_header(std::vector<unsigned char> &binary);
std::vector<unsigned char> decode(std::vector<unsigned char> &image_binary);

template <typename FH>
void encode(FH &image_header, std::vector<unsigned char> &image_binary,
            std::string output_path) {
  std::ofstream output(output_path, std::ios::binary);
  uint8_t zero8 = 0;
  uint16_t zero16 = 0;
  int zero = 0;
  u_int8_t image_descriptor = 32;

  output.write(reinterpret_cast<const char *>(&zero8), 1);
  output.write(reinterpret_cast<const char *>(&zero8), 1);
  output.write(reinterpret_cast<const char *>(&zero8), 1);
  output.write(reinterpret_cast<const char *>(&zero), 5);
  output.write(reinterpret_cast<const char *>(&zero16), 2);
  output.write(reinterpret_cast<const char *>(&zero16), 2);
  output.write(reinterpret_cast<const char *>(&image_header.width), 2);
  output.write(reinterpret_cast<const char *>(&image_header.height), 2);
  output.write(reinterpret_cast<const char *>(&image_header.bits_per_pixel), 1);
  output.write(reinterpret_cast<const char *>(&image_descriptor), 1);
}

} // namespace tga
