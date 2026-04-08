#include "deflate.hpp"
#include <vector>
#include <math.h>
#include <algorithm>
#include <map>
#include <queue>

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

    return compressed;
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

void build_codes(deflate::HuffmanNode *node, std::map<int, std::pair<int, int>> &codes, int code, int length)
{
    if (node == nullptr)
        return;

    if (!node->left && !node->right)
    {
        codes[node->value] = {code, length};
        return;
    }

    build_codes(node->left, codes, code << 1, length + 1);
    build_codes(node->right, codes, (code << 1) | 1, length + 1);
}

std::map<int, std::pair<int, int>> make_canonical(std::map<int, std::pair<int, int>> &codes)
{
    // Extract bit lengths
    std::map<int, int> bit_lengths;
    int max_len = 0;
    for (auto &[sym, pair] : codes)
    {
        bit_lengths[sym] = pair.second;
        max_len = std::max(max_len, pair.second);
    }

    // Count codes of each length
    std::vector<int> bl_count(max_len + 1, 0);
    for (auto &[sym, len] : bit_lengths)
        bl_count[len]++;
    bl_count[0] = 0;

    // Starting code for each length
    std::vector<int> next_code(max_len + 1, 0);
    int code = 0;
    for (int bits = 1; bits <= max_len; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    // Assign codes in symbol order (std::map iterates in order)
    std::map<int, std::pair<int, int>> canonical;
    for (auto &[sym, len] : bit_lengths)
    {
        canonical[sym] = {next_code[len], len};
        next_code[len]++;
    }
    return canonical;
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

struct CodeInfo { int base; int extra; };

// Index = length_code - 257
static const CodeInfo len_info[] = {
    {3,0},{4,0},{5,0},{6,0},{7,0},{8,0},{9,0},{10,0},
    {11,1},{13,1},{15,1},{17,1},
    {19,2},{23,2},{27,2},{31,2},
    {35,3},{43,3},{51,3},{59,3},
    {67,4},{83,4},{99,4},{115,4},
    {131,5},{163,5},{195,5},{227,5},
    {258,0}
};

// Index = distance_code
static const CodeInfo dist_info[] = {
    {1,0},{2,0},{3,0},{4,0},
    {5,1},{7,1},{9,2},{13,2},
    {17,3},{25,3},{33,4},{49,4},
    {65,5},{97,5},{129,6},{193,6},
    {257,7},{385,7},{513,8},{769,8},
    {1025,9},{1537,9},{2049,10},{3073,10},
    {4097,11},{6145,11},{8193,12},{12289,12},
    {16385,13},{24577,13}
};

std::vector<unsigned char> deflate(std::vector<unsigned char> &binary)
{
    int window_size = deflate::WINDOW_SIZE;
    if (binary.size() <= 16384)
        window_size = std::pow(2, std::floor(std::sqrt(binary.size())));
    std::vector<deflate::LZ77> lz_compressed = deflate::compress(binary, window_size);

    std::map<int, int> literal_freq;  // Standard size: (0-255) Byte - (256) End of Block - (257-285) Length Codes (Grouped)
    std::map<int, int> distance_freq; // Standard size (Grouped)

    for (auto &token : lz_compressed)
    {
        if (token.L > 2)
        {
            literal_freq[length_to_code(token.L)]++;
            distance_freq[distance_to_code(token.B)]++;
        }
        else
        {
            literal_freq[token.C]++;
        }
    }
    literal_freq[256] = 1; // End of Block

    auto cmp = [](deflate::HuffmanNode *a, deflate::HuffmanNode *b)
    { return a->freq > b->freq; }; // If a has higher frequency, push it down (weird syntax ngl)

    std::priority_queue<deflate::HuffmanNode *, std::vector<deflate::HuffmanNode *>, decltype(cmp)> lit_pq(cmp); // This is why C++ is hated, extremely unnecessary complexity
    std::priority_queue<deflate::HuffmanNode *, std::vector<deflate::HuffmanNode *>, decltype(cmp)> dist_pq(cmp);

    for (auto &[symbol, count] : literal_freq)
        lit_pq.push(new deflate::HuffmanNode(symbol, count));

    for (auto &[symbol, count] : distance_freq)
        dist_pq.push(new deflate::HuffmanNode(symbol, count));

    /* Literal Queue */
    while (lit_pq.size() >= 2)
    {
        // Left Node
        deflate::HuffmanNode *l = lit_pq.top();
        lit_pq.pop();

        // Right Node
        deflate::HuffmanNode *r = lit_pq.top();
        lit_pq.pop();

        deflate::HuffmanNode *new_node = new deflate::HuffmanNode(l->value + r->value, l->freq + r->freq);
        new_node->left = l;
        new_node->right = r;
        lit_pq.push(new_node);
    }

    deflate::HuffmanNode *lit_root = lit_pq.top();
    std::map<int, std::pair<int, int>> lit_codes;
    build_codes(lit_root, lit_codes, 0, 0);
    lit_codes = make_canonical(lit_codes);

    /* Distance Queue */
    while (dist_pq.size() >= 2)
    {
        // Left Node
        deflate::HuffmanNode *l = dist_pq.top();
        dist_pq.pop();

        // Right Node
        deflate::HuffmanNode *r = dist_pq.top();
        dist_pq.pop();

        deflate::HuffmanNode *new_node = new deflate::HuffmanNode(l->value + r->value, l->freq + r->freq);
        new_node->left = l;
        new_node->right = r;
        dist_pq.push(new_node);
    }

    deflate::HuffmanNode *dist_root = dist_pq.top();
    std::map<int, std::pair<int, int>> dist_codes;
    build_codes(dist_root, dist_codes, 0, 0);
    dist_codes = make_canonical(dist_codes);

    std::vector<unsigned char> deflated_binary;
    unsigned char current_byte = 0;
    int bit_pos = 0;

    // MSB-first: for Huffman codes
    auto write_bits = [&](int code, int length)
    {
        for (int i = length - 1; i >= 0; i--)
        {
            if (code & (1 << i))
                current_byte |= (1 << bit_pos);
            bit_pos++;
            if (bit_pos == 8)
            {
                deflated_binary.push_back(current_byte);
                current_byte = 0;
                bit_pos = 0;
            }
        }
    };

    // LSB-first: for extra bits and data values
    auto write_value = [&](int value, int num_bits)
    {
        for (int i = 0; i < num_bits; i++)
        {
            if (value & (1 << i))
                current_byte |= (1 << bit_pos);
            bit_pos++;
            if (bit_pos == 8)
            {
                deflated_binary.push_back(current_byte);
                current_byte = 0;
                bit_pos = 0;
            }
        }
    };

    for (int i = 0; i < lz_compressed.size(); i++)
    {
        if (lz_compressed[i].L > 2)
        {
            int lcode = length_to_code(lz_compressed[i].L);
            auto [lc, ll] = lit_codes[lcode];
            write_bits(lc, ll);
            if (len_info[lcode - 257].extra > 0)
                write_value(lz_compressed[i].L - len_info[lcode - 257].base, len_info[lcode - 257].extra);

            int dcode = distance_to_code(lz_compressed[i].B);
            auto [dc, dl] = dist_codes[dcode];
            write_bits(dc, dl);
            if (dist_info[dcode].extra > 0)
                write_value(lz_compressed[i].B - dist_info[dcode].base, dist_info[dcode].extra);

            auto [cc, cl] = lit_codes[lz_compressed[i].C];
            write_bits(cc, cl);
        }
        else
        {
            auto [cc, cl] = lit_codes[lz_compressed[i].C];
            write_bits(cc, cl);
        }
    }
    auto [ec, el] = lit_codes[256];
    write_bits(ec, el);
    if (bit_pos > 0)
        deflated_binary.push_back(current_byte);

    return deflated_binary;
}

std::vector<unsigned char> inflate(std::vector<unsigned char> &binary)
{
    for (auto &b : binary)
    {
    }
}
