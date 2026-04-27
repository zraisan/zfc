#include "jpg.hpp"
#include <vector>
#include <iostream>

jpg::FileHeader jpg::read_header(const std::vector<unsigned char> &binary)
{
    jpg::FileHeader header;
    header.offset = (binary[4] | (binary[5] << 8)) + 4; // Add 4 to account for signature
    header.width = binary[14] | (binary[15] << 8);
    header.height = binary[16] | (binary[17] << 8);

    // Find SOF marker (0xFFC0 - 0xFFC9, but typically SOF0)
    int idx = 2; // Start after SOI marker
    while (idx < binary.size() - 9)
    {
        if (binary[idx] == 0xFF)
        {
            uint8_t marker = binary[idx + 1];

            // Check if it's a SOF marker
            if ((marker >= 0xC0 && marker <= 0xC3) ||
                (marker >= 0xC5 && marker <= 0xC7) ||
                (marker >= 0xC9 && marker <= 0xCB))
            {

                // SOF marker found
                header.bits_per_pixel = binary[idx + 4]; // Precision (usually 8)
                header.channels = binary[idx + 9];       // Components
                break;
            }

            // Skip to next marker
            uint16_t length = (binary[idx + 2] << 8) | binary[idx + 3];
            idx += length + 2;
        }
        else
        {
            idx++;
        }
    }

    return header;
}

std::vector<unsigned char> decode(std::vector<unsigned char> &binary)
{
    try
    {
        // Check Signature
        if (std::string(binary.begin(), binary.begin() + 2) != "FFD8")
            throw -1;

        // Check Default Header
        if (std::string(binary.begin(), binary.begin() + 2) != "FFE0")
            throw -1;

        int idx = 0;
        jpg::FileHeader header = jpg::read_header(binary);
        idx += header.offset;
        std::vector<unsigned char> image_data;
        while (idx < binary.size())
        {
            if (binary[idx] == 0xDB) // DQT marker
            {
                int length = (binary[idx + 1] << 8) | binary[idx + 2];
                idx += length + 3;
            }
        }
    }
    catch (int e)
    {
        std::cout << "Error: This is not a valid JPG format";
    }
}
