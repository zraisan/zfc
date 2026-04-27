#include "jpg_dct.hpp"
#include <cmath>

double *dct::dct_transform(const std::vector<unsigned char> &block)
{
    double *dct_block = new double[64]; // I don't want to initialize a 2D array
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            double sum = 0.0;
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    sum += block[x * 8 + y] *
                           cos((2 * x + 1) * i * M_PI / (2 * 8)) *
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
