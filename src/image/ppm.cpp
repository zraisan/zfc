#include "ppm.hpp"
#include <bit>
#include <string>

ppm::FileHeader ppm::readHeader(const std::vector<unsigned char> &binary) {
  ppm::FileHeader fileHeader;
  fileHeader.fileType[0] = binary[0];
  fileHeader.fileType[1] = binary[1];

  int pos = 3;

  /* Ignore Coments */
  std::string comments;
  while (binary[pos] == '#') {
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

  fileHeader.width = std::stoi(w);
  fileHeader.height = std::stoi(h);

  std::string colorDepth;
  while (binary[pos] != '\n')
    colorDepth += binary[pos++];
  fileHeader.bitsPerPixel =
      (std::bit_width(static_cast<unsigned>(std::stoi(colorDepth)))) * 3;
  pos++;

  fileHeader.offset = pos;

  return fileHeader;
}

std::vector<unsigned char>
ppm::readImageData(std::vector<unsigned char> &imageBinary) {
  ppm::FileHeader imageHeader = ppm::readHeader(imageBinary);

  std::vector<unsigned char> imageData;

  for (int y = 0; y < imageHeader.height; y++) {
    for (int x = 0; x < imageHeader.width * 3; x++) {
      int i = y * imageHeader.width * 3 + x;
      imageData.push_back(imageBinary[imageHeader.offset + i]);
    }
  }
  return imageData;
}
