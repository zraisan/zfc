#include "png_filter.hpp"
#include <cstdint>
#include <cstdlib>

uint8_t filter::filter_sub(uint8_t x, uint8_t a) { return x - a; }

uint8_t filter::filter_up(uint8_t x, uint8_t b) { return x - b; }

uint8_t filter::filter_average(uint8_t x, uint8_t a, uint8_t b)
{
  return x - (a + b) / 2;
}

uint8_t filter::filter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
  return x - filter::paeth_predictor(a, b, c);
}

uint8_t filter::defilter_sub(uint8_t x, uint8_t a) { return x + a; }

uint8_t filter::defilter_up(uint8_t x, uint8_t b) { return x + b; }

uint8_t filter::defilter_average(uint8_t x, uint8_t a, uint8_t b)
{
  return x + (a + b) / 2;
}

uint8_t filter::defilter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c)
{
  return x + filter::paeth_predictor(a, b, c);
}

uint8_t filter::paeth_predictor(uint8_t a, uint8_t b, uint8_t c)
{
  int p = int(a) + int(b) - int(c);
  int pa = std::abs(p - int(a));
  int pb = std::abs(p - int(b));
  int pc = std::abs(p - int(c));
  if (pa <= pb && pa <= pc)
    return a;
  if (pb <= pc)
    return b;
  return c;
}
