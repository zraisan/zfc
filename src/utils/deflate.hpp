#pragma once

#include <cstdint>

namespace deflate
{
    uint8_t filterNone(uint8_t x);
    uint8_t filterSub(uint8_t x, uint8_t a);
    uint8_t filterUp(uint8_t x, uint8_t b);
    uint8_t filterAverage(uint8_t x, uint8_t a, uint8_t b);
    uint8_t filterPaeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c);
    uint8_t defilterNone(uint8_t x);
    uint8_t defilterSub(uint8_t x, uint8_t a);
    uint8_t defilterUp(uint8_t x, uint8_t b);
    uint8_t defilterAverage(uint8_t x, uint8_t a, uint8_t b);
    uint8_t defilterPaeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c);
    uint8_t paethPredictor(uint8_t a, uint8_t b, uint8_t c);
}
