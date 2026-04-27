#include "jpg_dct.hpp"
#include <algorithm>
#include <cmath>

double *dct::dct_transform(const std::vector<unsigned char> &block) {
    double *dct_block = new double[64];  // I don't want to initialize a 2D array
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double sum = 0.0;
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    sum += block[x * 8 + y] * cos((2 * x + 1) * i * M_PI / (2 * 8)) *
                           cos((2 * y + 1) * j * M_PI / (2 * 8));
                }
            }
            double ci = (i == 0) ? (1.0 / sqrt(8)) : (sqrt(2.0 / 8));
            double cj = (j == 0) ? (1.0 / sqrt(8)) : (sqrt(2.0 / 8));
            dct_block[i * 8 + j] = ci * cj * sum;
        }
    }
    return dct_block;
}

std::vector<unsigned char> dct::idct_transform(const std::vector<unsigned char> &dct_block) {
    std::vector<unsigned char> block(64);

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            double sum = 0.0;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    double ci = (i == 0) ? (1.0 / sqrt(8)) : (sqrt(2.0 / 8));
                    double cj = (j == 0) ? (1.0 / sqrt(8)) : (sqrt(2.0 / 8));
                    // Convert unsigned char DCT coefficient to double
                    double coeff = static_cast<double>(dct_block[i * 8 + j]);
                    sum += ci * cj * coeff * cos((2 * x + 1) * i * M_PI / (2 * 8)) *
                           cos((2 * y + 1) * j * M_PI / (2 * 8));
                }
            }
            // Clamp to 0-255 range and convert to unsigned char
            int pixel = static_cast<int>(round(sum));
            block[x * 8 + y] = static_cast<unsigned char>(std::max(0, std::min(255, pixel)));
        }
    }
    return block;
}
