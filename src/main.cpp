#include "image/image.hpp"
#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Error: Not enough arguments passed" << std::endl;
    return 1;
  }

  if (std::strcmp(argv[1], "image") == 0) {
    process_image(argv[2], argv[3]);
  } else if (std::strcmp(argv[1], "doc") == 0) {
    std::cout << "doc" << std::endl;
  } else if (std::strcmp(argv[1], "audio") == 0) {
    std::cout << "audio" << std::endl;
  } else {
    std::cout << "Error: Need to pass a valid document type (image, doc, audio)"
              << std::endl;
    return 1;
  }

  return 0;
}
