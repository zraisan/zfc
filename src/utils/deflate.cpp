#include "deflate.hpp"
#include <cstdint>
#include <math.h>

uint8_t deflate::filter_none(uint8_t x)
{
    return x;
}

uint8_t deflate::filter_sub(uint8_t x, uint8_t a)
{
    return x - a;
}

uint8_t deflate::filter_up(uint8_t x, uint8_t b)
{
    return x - b;
}

uint8_t deflate::filter_average(uint8_t x, uint8_t a, uint8_t b)
{
    return x - std::floor((a + b) / 2);
}

uint8_t deflate::filter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x - deflate::paeth_predictor(a, b, c);
}

uint8_t deflate::defilter_none(uint8_t x)
{
    return x;
}

uint8_t deflate::defilter_sub(uint8_t x, uint8_t a)
{
    return x + a;
}

uint8_t deflate::defilter_up(uint8_t x, uint8_t b)
{
    return x + b;
}

uint8_t deflate::defilter_average(uint8_t x, uint8_t a, uint8_t b)
{
    return x + std::floor((a + b) / 2);
}

uint8_t deflate::defilter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x + deflate::paeth_predictor(a, b, c);
}

uint8_t deflate::paeth_predictor(uint8_t a, uint8_t b, uint8_t c)
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
