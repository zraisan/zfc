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
  FileHeader imageHeader;
  if (inputType == "bmp") {
    imageHeader = readHeader(imageBinary);
    imageData = readImageData(imageBinary);
    std::cout << imageHeader.fileType << std::endl
              << imageHeader.fileSize << std::endl
              << imageHeader.width << std::endl
              << imageHeader.height << std::endl
              << imageHeader.offset << std::endl;
  }

  if (outputType == "ppm") {
    generate(imageHeader, imageData, outputPath);
  }
}
