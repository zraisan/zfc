#include "bmp.hpp"
#include "ppm.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<unsigned char> readImageBinary(std::string path) {
  std::ifstream input(path, std::ios::binary);
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
  return buffer;
}

std::string identifyType(std::string path) {
  size_t pos = path.find('.');
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(pos + 1);
}

void processImage(std::string inputPath, std::string outputPath) {
  std::string inputType = identifyType(inputPath);
  std::string outputType = identifyType(outputPath);
  std::cout << inputType << std::endl;
  std::vector<unsigned char> imageBinary = readImageBinary(inputPath);
  std::vector<unsigned char> imageData;
  int32_t width = 0, height = 0;
  uint16_t bitsPerPixel = 0;

  if (inputType == "bmp") {
    bmp::FileHeader header = bmp::readHeader(imageBinary);
    imageData = bmp::readImageData(imageBinary);
    width = header.width;
    height = header.height;
    bitsPerPixel = header.bitsPerPixel;
  } else if (inputType == "ppm") {
    ppm::FileHeader header = ppm::readHeader(imageBinary);
    imageData = ppm::readImageData(imageBinary);
    width = header.width;
    height = header.height;
    bitsPerPixel = header.bitsPerPixel;
  }

  if (outputType == "ppm") {
    bmp::FileHeader out = {{}, 0, 0, width, height, bitsPerPixel};
    ppm::generate(out, imageData, outputPath);
  } else if (outputType == "bmp") {
    uint32_t paddedRow = (width * 3 + 3) & ~3;
    uint32_t fileSize = 54 + paddedRow * height;
    bmp::FileHeader out = {{}, fileSize, 54, width, height, bitsPerPixel};
    bmp::generate(out, imageData, outputPath);
  }
}
