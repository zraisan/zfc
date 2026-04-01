#include "ppm.hpp"
#include <bit>
#include <string>

ppm::FileHeader ppm::read_header(const std::vector<unsigned char> &binary)
{
  ppm::FileHeader file_header;
  file_header.file_type[0] = binary[0];
  file_header.file_type[1] = binary[1];

  int pos = 3;

  /* Ignore Coments */
  std::string comments;
  while (binary[pos] == '#')
  {
    while (binary[pos] != '\n')
      pos++;
    pos++;
  }

  /* Parse String to Get Weight And Height */
  std::string w;
  while (binary[pos] != ' ')
    w += binary[pos++];
  pos++; // skip space
  // parse height
  std::string h;
  while (binary[pos] != '\n')
    h += binary[pos++];
  pos++;

  file_header.width = std::stoi(w);
  file_header.height = std::stoi(h);

  std::string color_depth;
  while (binary[pos] != '\n')
    color_depth += binary[pos++];
  file_header.bits_per_pixel =
      (std::bit_width(static_cast<unsigned>(std::stoi(color_depth)))) * 3;
  pos++;

  file_header.offset = pos;
  file_header.channels = 3;

  return file_header;
}

std::vector<unsigned char>
ppm::decode(std::vector<unsigned char> &image_binary)
{
  ppm::FileHeader image_header = ppm::read_header(image_binary);

  std::vector<unsigned char> image_data;

  for (int y = 0; y < image_header.height; y++)
  {
    for (int x = 0; x < image_header.width * 3; x++)
    {
      int i = y * image_header.width * 3 + x;
      image_data.push_back(image_binary[image_header.offset + i]);
    }
  }
  return image_data;
}
