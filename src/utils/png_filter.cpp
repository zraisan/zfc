#include "png_filter.hpp"
#include <cstdint>
#include <math.h>

uint8_t filter::filter_none(uint8_t x)
{
    return x;
}

uint8_t filter::filter_sub(uint8_t x, uint8_t a)
{
    return x - a;
}

uint8_t filter ::filter_up(uint8_t x, uint8_t b)
{
    return x - b;
}

uint8_t filter ::filter_average(uint8_t x, uint8_t a, uint8_t b)
{
    return x - std::floor((a + b) / 2);
}

uint8_t filter ::filter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x - filter ::paeth_predictor(a, b, c);
}

uint8_t filter::defilter_none(uint8_t x)
{
    return x;
}

uint8_t filter::defilter_sub(uint8_t x, uint8_t a)
{
    return x + a;
}

uint8_t filter::defilter_up(uint8_t x, uint8_t b)
{
    return x + b;
}

uint8_t filter::defilter_average(uint8_t x, uint8_t a, uint8_t b)
{
    return x + std::floor((a + b) / 2);
}

uint8_t filter::defilter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x + filter::paeth_predictor(a, b, c);
}

uint8_t filter::paeth_predictor(uint8_t a, uint8_t b, uint8_t c)
{
    uint8_t p = a + b - c;
    uint8_t pa = std::abs(p - a);
    uint8_t pb = std::abs(p - b);
    uint8_t pc = std::abs(p - c);
    uint8_t pr;
    if (pa <= pb && pa <= pc)
        pr = a;
    else if (pb <= pc)
        pr = b;
    else
        pr = c;
    return pr;
}
