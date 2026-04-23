#include "deflate.hpp"
#include <algorithm>
#include <map>
#include <math.h>
#include <queue>
#include <vector>

std::vector<deflate::LZ77> deflate::compress(std::vector<unsigned char> &binary,
                                             int window_size)
{
    int idx = 0;
    std::vector<deflate::LZ77> compressed;
    while (idx < binary.size())
    {
        int sw_start = std::max(0, idx - window_size);
        int best_offset = 0;
        int best_length = 2;
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
        if (best_offset > 0)
        {
            compressed.push_back({best_offset, best_length, 0});
            idx += best_length;
        }
        else
        {
            compressed.push_back({0, 0, binary[idx]});
            idx++;
        }
    }

    return compressed;
}

std::vector<unsigned char>
deflate::decompress(std::vector<deflate::LZ77> &compressed_binary)
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

void build_codes(deflate::HuffmanNode *node,
                 std::map<int, deflate::HuffmanCode> &codes, int code,
                 int length)
{
    if (node == nullptr)
        return;

    if (!node->left && !node->right)
    {
        codes[node->value] = deflate::HuffmanCode{code, length};
        return;
    }

    build_codes(node->left, codes, code << 1, length + 1);
    build_codes(node->right, codes, (code << 1) | 1, length + 1);
}

std::map<int, deflate::HuffmanCode>
make_canonical(std::map<int, deflate::HuffmanCode> &codes, int max_limit)
{
    // Extract bit lengths
    std::map<int, int> bit_lengths;
    int max_len = 0;
    for (auto &[sym, pair] : codes)
    {
        bit_lengths[sym] = std::min(pair.length, max_limit);
        max_len = std::max(max_len, bit_lengths[sym]);
    }

    // Count codes of each length
    std::vector<int> bl_count(max_len + 2, 0);
    for (auto &[sym, len] : bit_lengths)
        bl_count[len]++;
    bl_count[0] = 0;

    // If capping made the tree oversubscribed, lengthen the longest codes
    // (closest to max_len) until Kraft's inequality holds.
    long long kraft = 0;
    for (int L = 1; L <= max_len; L++)
        kraft += (long long)bl_count[L] << (max_len - L);
    long long target = 1LL << max_len;
    while (kraft > target)
    {
        for (int L = max_len - 1; L >= 1; L--)
        {
            if (bl_count[L] > 0)
            {
                bl_count[L]--;
                bl_count[L + 1]++;
                kraft -= 1LL << (max_len - L - 1);
                break;
            }
        }
    }

    // Reassign lengths to symbols: shorter current length → shorter new length
    std::vector<std::pair<int, int>> sorted_syms;
    for (auto &[sym, len] : bit_lengths)
        sorted_syms.push_back({len, sym});
    std::sort(sorted_syms.begin(), sorted_syms.end());
    {
        int idx = 0;
        for (int L = 1; L <= max_len; L++)
            for (int k = 0; k < bl_count[L]; k++)
                bit_lengths[sorted_syms[idx++].second] = L;
    }

    // Starting code for each length
    std::vector<int> next_code(max_len + 1, 0);
    int code = 0;
    for (int bits = 1; bits <= max_len; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    // Assign codes in symbol order (std::map iterates in order)
    std::map<int, deflate::HuffmanCode> canonical;
    for (auto &[sym, len] : bit_lengths)
    {
        canonical[sym] = deflate::HuffmanCode{next_code[len], len};
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
    // For dist >= 5: find highest bit b of (dist-1), then the next-highest
    // bit selects between the two codes 2b and 2b+1.
    int d = dist - 1;
    int b = 0;
    while ((1 << (b + 1)) <= d)
        b++;
    int second = (d >> (b - 1)) & 1;
    return 2 * b + second;
}

struct CodeInfo
{
    int base;
    int extra;
};

// Index = length_code - 257
static const CodeInfo len_info[] = {
    {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 1}, {13, 1}, {15, 1}, {17, 1}, {19, 2}, {23, 2}, {27, 2}, {31, 2}, {35, 3}, {43, 3}, {51, 3}, {59, 3}, {67, 4}, {83, 4}, {99, 4}, {115, 4}, {131, 5}, {163, 5}, {195, 5}, {227, 5}, {258, 0}};

// Index = distance_code
static const CodeInfo dist_info[] = {
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {7, 1}, {9, 2}, {13, 2}, {17, 3}, {25, 3}, {33, 4}, {49, 4}, {65, 5}, {97, 5}, {129, 6}, {193, 6}, {257, 7}, {385, 7}, {513, 8}, {769, 8}, {1025, 9}, {1537, 9}, {2049, 10}, {3073, 10}, {4097, 11}, {6145, 11}, {8193, 12}, {12289, 12}, {16385, 13}, {24577, 13}};

std::vector<unsigned char>
deflate::deflate(std::vector<unsigned char> &binary)
{
    int window_size = deflate::WINDOW_SIZE;
    if (binary.size() <= 16384)
        window_size = std::pow(2, std::floor(std::sqrt(binary.size())));
    std::vector<deflate::LZ77> lz_compressed =
        deflate::compress(binary, window_size);

    std::map<int, int> literal_freq;  // Standard size: (0-255) Byte - (256) End of
                                      // Block - (257-285) Length Codes (Grouped)
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
    if (distance_freq.empty())
        distance_freq[0] = 1; // DEFLATE requires at least one distance code

    auto cmp = [](deflate::HuffmanNode *a, deflate::HuffmanNode *b)
    {
        return a->freq > b->freq;
    }; // If a has higher frequency, push it down (weird syntax ngl)

    std::priority_queue<deflate::HuffmanNode *,
                        std::vector<deflate::HuffmanNode *>, decltype(cmp)>
        lit_pq(cmp); // This is why C++ is hated, extremely unnecessary complexity
    std::priority_queue<deflate::HuffmanNode *,
                        std::vector<deflate::HuffmanNode *>, decltype(cmp)>
        dist_pq(cmp);

    for (auto &[symbol, count] : literal_freq)
        lit_pq.push(new deflate::HuffmanNode(symbol, count));

    for (auto &[symbol, count] : distance_freq)
        dist_pq.push(new deflate::HuffmanNode(symbol, count));

    /* Literal Tree */
    while (lit_pq.size() >= 2)
    {
        // Left Node
        deflate::HuffmanNode *l = lit_pq.top();
        lit_pq.pop();

        // Right Node
        deflate::HuffmanNode *r = lit_pq.top();
        lit_pq.pop();

        deflate::HuffmanNode *new_node =
            new deflate::HuffmanNode(l->value + r->value, l->freq + r->freq);
        new_node->left = l;
        new_node->right = r;
        lit_pq.push(new_node);
    }

    deflate::HuffmanNode *lit_root = lit_pq.top();
    std::map<int, deflate::HuffmanCode> lit_codes;
    build_codes(lit_root, lit_codes, 0, 0);
    if (lit_codes.size() == 1)
        lit_codes.begin()->second.length = 1;
    lit_codes = make_canonical(lit_codes, 15);

    /* Distance Tree */
    while (dist_pq.size() >= 2)
    {
        // Left Node
        deflate::HuffmanNode *l = dist_pq.top();
        dist_pq.pop();

        // Right Node
        deflate::HuffmanNode *r = dist_pq.top();
        dist_pq.pop();

        deflate::HuffmanNode *new_node =
            new deflate::HuffmanNode(l->value + r->value, l->freq + r->freq);
        new_node->left = l;
        new_node->right = r;
        dist_pq.push(new_node);
    }

    deflate::HuffmanNode *dist_root = dist_pq.top();
    std::map<int, deflate::HuffmanCode> dist_codes;
    build_codes(dist_root, dist_codes, 0, 0);
    if (dist_codes.size() == 1)
        dist_codes.begin()->second.length = 1;
    dist_codes = make_canonical(dist_codes, 15);

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

    // Block header: BFINAL=1 (last block), BTYPE=10 (dynamic Huffman)
    write_value(1, 1);
    write_value(2, 2);

    // D3: emit dynamic Huffman tree description

    // HLIT and HDIST: smallest legal counts that cover all used symbols.
    int max_lit = 256; // EOB always present, ensures hlit >= 257
    for (auto &[sym, p] : lit_codes)
        max_lit = std::max(max_lit, sym);
    int hlit = max_lit + 1; // 257..286

    int max_dist = 0;
    for (auto &[sym, p] : dist_codes)
        max_dist = std::max(max_dist, sym);
    int hdist = max_dist + 1; // 1..30

    // Flat code-length sequence: lit lengths [0..hlit-1] then dist lengths
    // [0..hdist-1]
    std::vector<int> all_lens(hlit + hdist, 0);
    for (auto &[sym, p] : lit_codes)
        all_lens[sym] = p.length;
    for (auto &[sym, p] : dist_codes)
        all_lens[hlit + sym] = p.length;

    // RLE-encode using the code-length alphabet:
    //  0-15 = literal length, 16 = repeat prev 3-6x (+2), 17 = 0-run 3-10 (+3),
    //  18 = 0-run 11-138 (+7).
    std::vector<deflate::RleSym> rle;
    {
        int i = 0, n = (int)all_lens.size();
        while (i < n)
        {
            int v = all_lens[i];
            int j = i;
            while (j < n && all_lens[j] == v)
                j++;
            int run = j - i;
            if (v == 0)
            {
                while (run >= 11)
                {
                    int take = std::min(run, 138);
                    rle.push_back({18, take - 11});
                    run -= take;
                }
                while (run >= 3)
                {
                    int take = std::min(run, 10);
                    rle.push_back({17, take - 3});
                    run -= take;
                }
                while (run > 0)
                {
                    rle.push_back({0, 0});
                    run--;
                }
            }
            else
            {
                rle.push_back({v, 0});
                run--;
                while (run >= 3)
                {
                    int take = std::min(run, 6);
                    rle.push_back({16, take - 3});
                    run -= take;
                }
                while (run > 0)
                {
                    rle.push_back({v, 0});
                    run--;
                }
            }
            i = j;
        }
    }

    // Build a Huffman tree over the code-length alphabet.
    std::map<int, int> clen_freq;
    for (auto &r : rle)
        clen_freq[r.sym]++;

    std::priority_queue<deflate::HuffmanNode *,
                        std::vector<deflate::HuffmanNode *>, decltype(cmp)>
        clen_pq(cmp);
    for (auto &[s, c] : clen_freq)
        clen_pq.push(new deflate::HuffmanNode(s, c));

    while (clen_pq.size() >= 2)
    {
        deflate::HuffmanNode *l = clen_pq.top();
        clen_pq.pop();
        deflate::HuffmanNode *r = clen_pq.top();
        clen_pq.pop();
        deflate::HuffmanNode *new_node =
            new deflate::HuffmanNode(l->value + r->value, l->freq + r->freq);
        new_node->left = l;
        new_node->right = r;
        clen_pq.push(new_node);
    }

    deflate::HuffmanNode *clen_root = clen_pq.top();
    std::map<int, deflate::HuffmanCode> clen_codes;
    build_codes(clen_root, clen_codes, 0, 0);
    if (clen_codes.size() == 1)
        clen_codes.begin()->second.length = 1;
    clen_codes = make_canonical(clen_codes, 7);

    // Emit HCLEN: code-length code lengths in the fixed permutation, with
    // trailing zeros trimmed (minimum 4 entries kept).
    static const int clen_perm[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
                                      11, 4, 12, 3, 13, 2, 14, 1, 15};
    int clen_lens[19] = {0};
    for (auto &[s, p] : clen_codes)
        clen_lens[s] = p.length;

    int hclen = 19;
    while (hclen > 4 && clen_lens[clen_perm[hclen - 1]] == 0)
        hclen--;

    write_value(hlit - 257, 5);
    write_value(hdist - 1, 5);
    write_value(hclen - 4, 4);
    for (int k = 0; k < hclen; k++)
        write_value(clen_lens[clen_perm[k]], 3);

    // Emit the RLE'd lit+dist code lengths using the code-length Huffman codes.
    for (auto &r : rle)
    {
        auto [c, l] = clen_codes[r.sym];
        write_bits(c, l);
        if (r.sym == 16)
            write_value(r.extra, 2);
        else if (r.sym == 17)
            write_value(r.extra, 3);
        else if (r.sym == 18)
            write_value(r.extra, 7);
    }

    for (int i = 0; i < lz_compressed.size(); i++)
    {
        if (lz_compressed[i].L >= 3)
        {
            int lcode = length_to_code(lz_compressed[i].L);
            auto [lc, ll] = lit_codes[lcode];
            write_bits(lc, ll);
            if (len_info[lcode - 257].extra > 0)
                write_value(lz_compressed[i].L - len_info[lcode - 257].base,
                            len_info[lcode - 257].extra);

            int dcode = distance_to_code(lz_compressed[i].B);
            auto [dc, dl] = dist_codes[dcode];
            write_bits(dc, dl);
            if (dist_info[dcode].extra > 0)
                write_value(lz_compressed[i].B - dist_info[dcode].base,
                            dist_info[dcode].extra);
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

std::vector<unsigned char>
deflate::make_zlib(const std::vector<unsigned char> &original,
                   const std::vector<unsigned char> &deflated)
{
    std::vector<unsigned char> out;
    out.push_back(0x78); // CMF: deflate, 32K window
    out.push_back(0x9C); // FLG: default compression, checksum-valid
    out.insert(out.end(), deflated.begin(), deflated.end());

    uint32_t a = 1, b = 0;
    for (unsigned char c : original)
    {
        a = (a + c) % 65521;
        b = (b + a) % 65521;
    }
    uint32_t adler = (b << 16) | a;
    out.push_back((adler >> 24) & 0xff);
    out.push_back((adler >> 16) & 0xff);
    out.push_back((adler >> 8) & 0xff);
    out.push_back(adler & 0xff);
    return out;
}

// LSB-first bit reader, mirrors the byte packing used by deflate's writer.
int read_value(deflate::BitReader &br, int num_bits)
{
    int value = 0;
    for (int i = 0; i < num_bits; i++)
    {
        int bit = ((*br.data)[br.byte_idx] >> br.bit_pos) & 1;
        value |= (bit << i);
        br.bit_pos++;
        if (br.bit_pos == 8)
        {
            br.bit_pos = 0;
            br.byte_idx++;
        }
    }
    return value;
}

// Build a (code, length) symbol lookup from a list of canonical bit lengths.
std::map<std::pair<int, int>, int>
build_canonical(const std::vector<int> &lengths)
{
    std::map<std::pair<int, int>, int> table;
    int max_len = 0;
    for (int l : lengths)
        max_len = std::max(max_len, l);
    if (max_len == 0)
        return table;

    std::vector<int> bl_count(max_len + 1, 0);
    for (int l : lengths)
        if (l > 0)
            bl_count[l]++;

    std::vector<int> next_code(max_len + 1, 0);
    int code = 0;
    for (int bits = 1; bits <= max_len; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    for (int sym = 0; sym < (int)lengths.size(); sym++)
    {
        int len = lengths[sym];
        if (len > 0)
        {
            table[{next_code[len], len}] = sym;
            next_code[len]++;
        }
    }
    return table;
}

// Read bits MSB-first into a code accumulator until it matches a canonical
// code.
int decode_symbol(deflate::BitReader &br,
                  const std::map<std::pair<int, int>, int> &table)
{
    int code = 0;
    int len = 0;
    while (len < 16)
    {
        code = (code << 1) | read_value(br, 1);
        len++;
        auto it = table.find({code, len});
        if (it != table.end())
            return it->second;
    }
    return -1; // malformed stream
}

std::vector<unsigned char>
deflate::inflate(const std::vector<unsigned char> &binary)
{
    deflate::BitReader br{&binary, 0, 0};
    std::vector<unsigned char> output;
    bool last_block = false;

    while (!last_block)
    {
        int bfinal = read_value(br, 1);
        int btype = read_value(br, 2);
        last_block = (bfinal == 1);

        if (btype == 0)
        {
            // Stored block: byte-align, read LEN/NLEN, copy LEN raw bytes.
            if (br.bit_pos != 0)
            {
                br.bit_pos = 0;
                br.byte_idx++;
            }
            int len = (*br.data)[br.byte_idx] | ((*br.data)[br.byte_idx + 1] << 8);
            br.byte_idx += 4; // skip LEN (2 bytes) + NLEN (2 bytes)
            for (int i = 0; i < len; i++)
                output.push_back((*br.data)[br.byte_idx++]);
            continue;
        }

        std::map<std::pair<int, int>, int> lit_table, dist_table;

        if (btype == 1)
        {
            // Fixed Huffman: hardcoded length tables from RFC 1951 section 3.2.6.
            std::vector<int> lit_lens(288);
            for (int i = 0; i <= 143; i++)
                lit_lens[i] = 8;
            for (int i = 144; i <= 255; i++)
                lit_lens[i] = 9;
            for (int i = 256; i <= 279; i++)
                lit_lens[i] = 7;
            for (int i = 280; i <= 287; i++)
                lit_lens[i] = 8;
            std::vector<int> dist_lens(32, 5);
            lit_table = build_canonical(lit_lens);
            dist_table = build_canonical(dist_lens);
        }
        else if (btype == 2)
        {
            // Dynamic Huffman: read tree description (inverse of D3).
            int hlit = read_value(br, 5) + 257;
            int hdist = read_value(br, 5) + 1;
            int hclen = read_value(br, 4) + 4;

            static const int clen_perm[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
                                              11, 4, 12, 3, 13, 2, 14, 1, 15};
            std::vector<int> clen_lens(19, 0);
            for (int i = 0; i < hclen; i++)
                clen_lens[clen_perm[i]] = read_value(br, 3);

            auto clen_table = build_canonical(clen_lens);

            // Decode hlit + hdist code lengths using the code-length tree.
            std::vector<int> all_lens;
            while ((int)all_lens.size() < hlit + hdist)
            {
                int sym = decode_symbol(br, clen_table);
                if (sym < 16)
                {
                    all_lens.push_back(sym);
                }
                else if (sym == 16)
                {
                    int repeat = read_value(br, 2) + 3;
                    int prev = all_lens.back();
                    for (int j = 0; j < repeat; j++)
                        all_lens.push_back(prev);
                }
                else if (sym == 17)
                {
                    int repeat = read_value(br, 3) + 3;
                    for (int j = 0; j < repeat; j++)
                        all_lens.push_back(0);
                }
                else if (sym == 18)
                {
                    int repeat = read_value(br, 7) + 11;
                    for (int j = 0; j < repeat; j++)
                        all_lens.push_back(0);
                }
            }

            std::vector<int> lit_lens(all_lens.begin(), all_lens.begin() + hlit);
            std::vector<int> dist_lens(all_lens.begin() + hlit, all_lens.end());
            lit_table = build_canonical(lit_lens);
            dist_table = build_canonical(dist_lens);
        }
        else
        {
            return output; // BTYPE=11 is reserved/invalid
        }

        // Decode the compressed payload until end-of-block (shared by btype 1 & 2).
        while (true)
        {
            int sym = decode_symbol(br, lit_table);
            if (sym < 256)
            {
                output.push_back((unsigned char)sym);
            }
            else if (sym == 256)
            {
                break;
            }
            else
            {
                int lcode = sym - 257;
                int length = len_info[lcode].base;
                if (len_info[lcode].extra > 0)
                    length += read_value(br, len_info[lcode].extra);

                int dcode = decode_symbol(br, dist_table);
                int distance = dist_info[dcode].base;
                if (dist_info[dcode].extra > 0)
                    distance += read_value(br, dist_info[dcode].extra);

                int start = (int)output.size() - distance;
                for (int j = 0; j < length; j++)
                    output.push_back(output[start + j]);
            }
        }
    }

    return output;
}
