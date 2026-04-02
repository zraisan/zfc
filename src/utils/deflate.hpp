#pragma once

#include <vector>

namespace deflate
{
    struct LZ77
    {
        int B;
        int L;
        int C;
    };
    std::vector<LZ77> compress(std::vector<unsigned char> &binary, int sliding_window_size, int lookahead_buffer);
    std::vector<unsigned char> decompress(std::vector<unsigned char> &binary);
}
