#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace jpg {
    struct FileHeader {
        char file_type[2];
        uint32_t file_size;
        uint32_t offset;
        int32_t width;
        int32_t height;
        uint16_t bits_per_pixel;
        uint8_t channels;
    };
    FileHeader read_header(const std::vector<unsigned char> &binary);

    template<typename FH>
    std::vector<unsigned char> encode(FH &image_header, std::vector<unsigned char> &image_data,
                                      std::string output_path) {
        std::ofstream output(output_path, std::ios::binary);

        output.write("\xFF\xD8", 2);  // Start of Image
        output.write("\xFF\xD8", 2);  // Default Header
    }

    std::vector<unsigned char> decode(std::vector<unsigned char> &binary);
}  // namespace jpg