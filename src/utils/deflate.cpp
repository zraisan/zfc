#include "deflate.hpp"
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
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
        }
    }

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
    std::map<int, std::string> lit_codes;
    build_codes(lit_root, lit_codes, "");

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
    std::map<int, std::string> dist_codes;
    build_codes(dist_root, dist_codes, "");

    std::string deflated_concat = "";
    for (int i = 0; i < lz_compressed.size(); i++)
    {
        if (lz_compressed[i].L > 2) // In general it is cheaper to group length of 1 and 2 with 0
        {
            deflated_concat += lit_codes[lz_compressed[i].L];
            deflated_concat += dist_codes[lz_compressed[i].B];
            deflated_concat += lit_codes[lz_compressed[i].C];
        }
        else
        {
            deflated_concat += lit_codes[lz_compressed[i].C];
        }
    }
    deflated_concat += lit_codes[256];
    std::vector<unsigned char> deflated_binary(deflated_concat.begin(), deflated_concat.end());

    return deflated_binary;
}

std::vector<unsigned char> inflate(std::vector<unsigned char> &binary)
{
}

void build_codes(deflate::HuffmanNode *node, std::map<int, std::string> &codes, std::string code)
{
    if (node == nullptr)
        return;

    if (!node->left && !node->right)
    {
        codes[node->value] = code;
        return;
    }

    build_codes(node->left, codes, code + "0");
    build_codes(node->right, codes, code + "1");
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