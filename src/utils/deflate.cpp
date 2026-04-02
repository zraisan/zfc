#include "deflate.hpp"
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>

std::vector<deflate::LZ77> deflate::compress(std::vector<unsigned char> &binary, int window_size)
{
    int idx = 0;
    std::vector<deflate::LZ77> compressed;
    while (idx < binary.size())
    {
        int sw_start = std::max(0, idx - window_size);
        int best_offset = 0;
        int best_length = 0;
        for (int i = sw_start; i < idx; i++)
        {
            int length = 0;
            int max_len = std::min(MAX_MATCH_LENGTH, (int)binary.size() - idx);
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
            compressed.push_back({best_offset, best_length, binary[next_idx]});
            idx += best_length + 1;
        }
        else
        {
            compressed.push_back({0, 0, binary[idx]});
            idx++;
        }
    }
}

std::vector<unsigned char> deflate::decompress(std::vector<deflate::LZ77> &compressed_binary)
{
    std::vector<unsigned char> binary;
    for (int i = 0; i < compressed_binary.size(); i++)
    {
        if (compressed_binary[i].L > 0)
        {
            int start = binary.size() - compressed_binary[i].B;
            for (int j = 0; j < compressed_binary[i].L; j++)
                binary.push_back(binary[start + j]);
        }
        else
        {
            binary.push_back(compressed_binary[i].C);
        }
    }
    return binary;
}

std::vector<unsigned char> deflate(std::vector<unsigned char> &binary)
{
    int window_size = deflate::WINDOW_SIZE;
    if (binary.size() <= 16384)
        window_size = std::pow(2, std::floor(std::sqrt(binary.size())));
    std::vector<deflate::LZ77> lz_compressed = deflate::compress(binary, window_size);
    std::vector<unsigned char> literal_stream;
    std::vector<unsigned char> distance_stream;
    for (int i = 0; i < lz_compressed.size(); i++)
    {
        if (lz_compressed[i].L > 2) // In general it is cheaper to group length of 1 and 2 with 0
        {
            literal_stream.push_back(lz_compressed[i].L);
            literal_stream.push_back(lz_compressed[i].C);

            distance_stream.push_back(lz_compressed[i].B);
        }
        else
        {
            literal_stream.push_back(lz_compressed[i].C);

            distance_stream.push_back(lz_compressed[i].B);
        }
    }

    int literal_freq[286];  // Standard size: (0-255) Byte - (256) End of Block - (257-285) Length Codes (Grouped)
    int distance_freq[258]; // Standard size (Grouped)

    for (auto &token : lz_compressed)
    {
        if (token.L > 0)
        {
            literal_freq[token.L + 254]++;
        }
        else
        {
            literal_freq[token.C]++;
        }
    }
}

int length_to_code(int length)
{
    if (length <= 10)
        return 254 + length; // 257-264
    if (length <= 18)
        return 265 + (length - 11) / 2;
    if (length <= 34)
        return 269 + (length - 19) / 4;
    if (length <= 66)
        return 273 + (length - 35) / 8;
    if (length <= 130)
        return 277 + (length - 67) / 16;
    if (length <= 257)
        return 281 + (length - 131) / 32;
    return 285; // 258
}

int distance_to_code(int dist)
{
    if (dist <= 4)
        return dist - 1; // 0-3
    int code = 2;
    int d = dist - 1;
    while (d >= 2)
    {
        d >>= 1;
        code++;
    }
    return code;
}