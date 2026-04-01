#include "tga.hpp"
#include <vector>

tga::FileHeader tga::read_header(std::vector<unsigned char> &binary)
{
    tga::FileHeader header;
    header.file_id_length = binary[0];
    header.width = binary[12] | (binary[13] << 8);
    header.height = binary[14] | (binary[15] << 8);
    header.bits_per_pixel = binary[16];
    header.right_to_left = (binary[17] >> 4) & 1;
    header.top_to_bottom = (binary[17] >> 5) & 1;
    header.offset = 18 + header.file_id_length;
    header.channels = header.bits_per_pixel / 8;
    return header;
}

std::vector<unsigned char>
tga::decode(std::vector<unsigned char> &image_binary)
{
    tga::FileHeader header = tga::read_header(image_binary);
    int y;
    bool condition;
    int increment;
    std::vector<unsigned char> image_data;
    for (int y = 0; y < header.height; y++)
    {
        int row = header.top_to_bottom ? y : header.height - 1 - y;
        for (int x = 0; x < header.width * 3; x += 3)
        {
            int col = header.right_to_left ? x : header.width * 3 - 1 - x;
            int r = col + 2;
            int g = col + 1;
            int b = col;
            image_data.push_back(image_binary[header.offset + r]);
            image_data.push_back(image_binary[header.offset + g]);
            image_data.push_back(image_binary[header.offset + b]);
        }
    }
    return image_data;
}