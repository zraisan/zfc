#pragma once

#include <cstdint>

namespace deflate
{
    uint8_t filter_none(uint8_t x);
    uint8_t filter_sub(uint8_t x, uint8_t a);
    uint8_t filter_up(uint8_t x, uint8_t b);
    uint8_t filter_average(uint8_t x, uint8_t a, uint8_t b);
    uint8_t filter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c);
    uint8_t defilter_none(uint8_t x);
    uint8_t defilter_sub(uint8_t x, uint8_t a);
    uint8_t defilter_up(uint8_t x, uint8_t b);
    uint8_t defilter_average(uint8_t x, uint8_t a, uint8_t b);
    uint8_t defilter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c);
    uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);
}
