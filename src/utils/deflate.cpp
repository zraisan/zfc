#include "deflate.hpp"
#include <cstdint>
#include <math.h>

uint8_t deflate::filterNone(uint8_t x)
{
    return x;
}

uint8_t deflate::filterSub(uint8_t x, uint8_t a)
{
    return x - a;
}

uint8_t deflate::filterUp(uint8_t x, uint8_t b)
{
    return x - b;
}

uint8_t deflate::filterAverage(uint8_t x, uint8_t a, uint8_t b)
{
    return x - std::floor((a + b) / 2);
}

uint8_t deflate::filterPaeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x - deflate::paethPredictor(a, b, c);
}

uint8_t deflate::defilterNone(uint8_t x)
{
    return x;
}

uint8_t deflate::defilterSub(uint8_t x, uint8_t a)
{
    return x + a;
}

uint8_t deflate::defilterUp(uint8_t x, uint8_t b)
{
    return x + b;
}

uint8_t deflate::defilterAverage(uint8_t x, uint8_t a, uint8_t b)
{
    return x + std::floor((a + b) / 2);
}

uint8_t deflate::defilterPaeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
    return x + deflate::paethPredictor(a, b, c);
}

uint8_t deflate::paethPredictor(uint8_t a, uint8_t b, uint8_t c)
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
