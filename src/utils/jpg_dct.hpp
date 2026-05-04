#pragma once

#include <vector>

namespace dct {
    struct HuffmanTable {
        u_int16_t lh;
        u_int8_t tc_th;
        u_int8_t l;
        u_int8_t v;
    };

    double *dct_transform(const std::vector<unsigned char> &block);
    std::vector<unsigned char> idct_transform(const std::vector<unsigned char> &dct_block);
}  // namespace dct
