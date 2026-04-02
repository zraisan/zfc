#include "deflate.hpp"
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>

std::vector<deflate::LZ77> deflate::compress(std::vector<unsigned char> &binary, int sliding_window_size, int lookahead_buffer)
{
    int idx = 0;
    std::vector<deflate::LZ77> compressed;
    while (idx < binary.size())
    {
        int sw_start = std::max(0, idx - sliding_window_size);
        int best_offset = 0;
        int best_length = 0;
        for (int i = sw_start; i < idx; i++)
        {
            int length = 0;
            int max_len = std::min(lookahead_buffer, (int)binary.size() - idx);
            while (length < max_len && binary[i + length] == binary[idx + length])
            {
                length++;
            }
            if (length > best_length)
            {
                best_offset = idx - i;
                best_length = length;
            }
        }
        if (best_length > 0)
        {
            int next_idx = idx + best_length;
            unsigned char next_char = next_idx < (int)binary.size() ? binary[next_idx] : 0;
            compressed.push_back({best_offset, best_length, next_char});
            idx += best_length + 1;
        }
        else
        {
            compressed.push_back({0, 0, binary[idx]});
            idx++;
        }
    }
}
