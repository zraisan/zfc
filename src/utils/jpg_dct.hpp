#pragma once

#include <vector>

namespace dct {
    double *dct_transform(const std::vector<unsigned char> &block);
    std::vector<unsigned char> idct_transform(const std::vector<unsigned char> &dct_block);
}  // namespace dct
