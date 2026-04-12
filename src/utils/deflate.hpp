#pragma once

#include <vector>
#include <cstdint>

namespace deflate
{
    struct LZ77
    {
        int B; // How many bytes back in the window the match starts
        int L; // How many bytes to copy from that match
        int C; // The next literal byte that didn't fit into the match
    };
    struct HuffmanNode
    {
        uint16_t value;
        int freq;
        HuffmanNode *left;
        HuffmanNode *right;
        HuffmanNode(uint16_t v, int freq, HuffmanNode *left = nullptr, HuffmanNode *right = nullptr)
            : value(v), freq(freq), left(left), right(right)
        {
        }
    };
    struct RleSym
    {
        int sym;   // code-length alphabet symbol (0-18)
        int extra; // value for the extra bits (0 if sym < 16)
    };
    struct BitReader
    {
        const std::vector<unsigned char> *data;
        size_t byte_idx;
        int bit_pos;
    };
    const int WINDOW_SIZE = 32768;
    const int MAX_MATCH_LENGTH = 258;

    std::vector<LZ77> compress(std::vector<unsigned char> &binary, int window_size = WINDOW_SIZE);
    std::vector<unsigned char> decompress(std::vector<LZ77> &compressed_binary);
    std::vector<unsigned char> deflate(std::vector<unsigned char> &binary);
    std::vector<unsigned char> inflate(std::vector<unsigned char> &binary);
    std::vector<unsigned char> make_zlib(const std::vector<unsigned char> &original,
                                         const std::vector<unsigned char> &deflated);
}
