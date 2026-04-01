#include "bmp.hpp"
#include "ppm.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<unsigned char> read_image_binary(std::string path) {
  std::ifstream input(path, std::ios::binary);
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
  return buffer;
}

std::string identify_type(std::string path) {
  size_t pos = path.find('.');
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(pos + 1);
}

void process_image(std::string input_path, std::string output_path) {
  std::string input_type = identify_type(input_path);
  std::string output_type = identify_type(output_path);
  std::cout << input_type << std::endl;
  std::vector<unsigned char> image_binary = read_image_binary(input_path);
  std::vector<unsigned char> image_data;
  int32_t width = 0, height = 0;
  uint16_t bits_per_pixel = 0;
  uint8_t channels = 3;

  if (input_type == "bmp") {
    bmp::FileHeader header = bmp::read_header(image_binary);
    image_data = bmp::decode(image_binary);
    width = header.width;
    height = header.height;
    bits_per_pixel = header.bits_per_pixel;
    channels = header.channels;
  } else if (input_type == "ppm") {
    ppm::FileHeader header = ppm::read_header(image_binary);
    image_data = ppm::decode(image_binary);
    width = header.width;
    height = header.height;
    bits_per_pixel = header.bits_per_pixel;
    channels = header.channels;
  }

  if (output_type == "ppm") {
    ppm::FileHeader out = {{}, 0, 0, width, height, bits_per_pixel, 3};
    ppm::encode(out, image_data, output_path);
  } else if (output_type == "bmp") {
    uint32_t padded_row = channels == 4 ? width * 4 : (width * 3 + 3) & ~3;
    uint32_t file_size = 54 + padded_row * height;
    bmp::FileHeader out = {{}, file_size, 54, width, height, bits_per_pixel, channels};
    bmp::encode(out, image_data, output_path);
  }
}
