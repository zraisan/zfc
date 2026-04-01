#pragma once

#include <string>
#include <vector>

std::vector<unsigned char> read_image_binary(std::string path);
std::string identify_type(std::string path);
void process_image(std::string input_path, std::string output_path);
