#include "jpg.hpp"
#include <iostream>
#include <vector>
#include "jpg_dct.hpp"

jpg::FileHeader jpg::read_header(const std::vector<unsigned char> &binary) {
    jpg::FileHeader header;
    header.offset =
        ((binary[4] << 8) | binary[5]) + 4;  // BE length + 4 bytes for SOI + APP0 marker

    // Find SOF marker (0xFFC0 - 0xFFC9, but typically SOF0)
    int idx = 2;  // Start after SOI marker
    while (idx < binary.size() - 9) {
        if (binary[idx] == 0xFF) {
            uint8_t marker = binary[idx + 1];

            // Check if it's a SOF marker
            if ((marker >= 0xC0 && marker <= 0xC3) || (marker >= 0xC5 && marker <= 0xC7) ||
                (marker >= 0xC9 && marker <= 0xCB)) {
                // SOF marker found
                header.bits_per_pixel = binary[idx + 4];  // Precision (8 ot 12)
                header.height = binary[5] | (binary[6] << 8);
                header.width = binary[7] | (binary[8] << 8);
                header.channels = binary[idx + 9];  // Components

                break;
            }

            // Skip to next marker
            uint16_t length = (binary[idx + 2] << 8) | binary[idx + 3];
            idx += length + 2;
        } else {
            idx++;
        }
    }

    return header;
}

std::vector<unsigned char> jpg::decode(std::vector<unsigned char> &binary) {
    try {
        std::vector<unsigned char> decoded_image;
        // Check Signature (SOI marker FF D8 at the start)
        if (binary[0] != 0xFF || binary[1] != 0xD8)
            throw -1;

        // Check Default Header (APP0 marker FF E0 right after SOI)
        if (binary[2] != 0xFF || binary[3] != 0xE0)
            throw -1;

        int idx = 0;
        jpg::FileHeader header = jpg::read_header(binary);
        idx += header.offset;
        std::vector<unsigned char> image_data;
        while (idx < binary.size()) {
            if (binary[idx] == 0xDB)  // DCT marker
            {
                int length = (binary[idx + 1] << 8) | binary[idx + 2];
                idx += length + 3;
                while (idx < length) {
                    // Create a sub-vector of 64 bytes starting at current idx position
                    std::vector<unsigned char> dct_block(image_data.begin() + idx,
                                                         image_data.begin() + idx + 64);
                    std::vector<unsigned char> spatial_block = dct::idct_transform(dct_block);
                    decoded_image.insert(decoded_image.end(), spatial_block.begin(),
                                         spatial_block.end());
                    idx += 64;
                }
            }
            if (binary[idx] == 0xC0)  // Start of frame
            {
                int length = (binary[idx + 1] << 8) | binary[idx + 2];
                idx += length + 3;
            }
        }
    } catch (int e) {
        std::cout << "Error: This is not a valid JPG format";
    }
}
