#pragma once

#include <string>
#include <vector>

std::vector<unsigned char> readImageBinary(std::string path);
std::string identifyType(std::string path);
void processImage(std::string inputPath, std::string outputPath);
