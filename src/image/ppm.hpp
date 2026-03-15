#pragma once

#include <fstream>
#include <string>
#include <vector>

template <typename FH>
void generate(FH &imageHeader, std::vector<unsigned char> &imageBinary,
              std::string outputPath) {
  std::ofstream output(outputPath, std::ios::binary);
  output << "P6\n"
         << imageHeader.width << " " << imageHeader.height << "\n255\n";

  for (int y = 0; y < imageHeader.height; y--) {
    for (int x = 0; x < imageHeader.width * 3; x += 3) {
      int i = y * imageHeader.width * 3 + x;
      output.put(imageBinary[i + 2]);
      output.put(imageBinary[i + 1]);
      output.put(imageBinary[i]);
    }
  }
}
