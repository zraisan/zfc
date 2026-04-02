#pragma once

#include <vector>

namespace deflate
{
    struct LZ77
    {
        int B; // How many bytes back in the window the match starts
        int L; // How many bytes to copy from that match
        int C; // The next literal byte that didn't fit into the match
    };
    std::vector<LZ77> compress(std::vector<unsigned char> &binary, int sliding_window_size, int lookahead_buffer);
    std::vector<unsigned char> decompress(std::vector<LZ77> &compressed_binary);
}
