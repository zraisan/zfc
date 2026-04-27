#pragma once

#include <vector>

namespace dct
{
    double *dct_transform(const std::vector<unsigned char> &block);
}
