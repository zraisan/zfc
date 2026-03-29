#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace qoi {
struct FileHeader {
  uint32_t width;
  uint32_t height;
  uint8_t channels;
  uint8_t colorspace;
  uint32_t offset;
};

typedef union {
  struct {
    unsigned char r, g, b, a;
  } rgba;
  uint32_t v;
} ColorIndexArray;

inline int getIndex(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
}

FileHeader readHeader(const std::vector<unsigned char> &binary);
std::vector<unsigned char>
readImageData(std::vector<unsigned char> &imageBinary);

template <typename FH>
void generate(FH &imageHeader, std::vector<unsigned char> &imageBinary,
              std::string outputPath) {
  std::ofstream output(outputPath, std::ios::binary);
  uint32_t width = imageHeader.width;
  uint32_t height = imageHeader.height;
  uint8_t channels = imageHeader.channels;
  uint8_t colorspace = 0;

  output.write("qoif", 4);
  output.write(reinterpret_cast<const char *>(&width), 4);
  output.write(reinterpret_cast<const char *>(&height), 4);
  output.write(reinterpret_cast<const char *>(&channels), 1);
  output.write(reinterpret_cast<const char *>(&colorspace), 1);

  int idx = 0;
  ColorIndexArray index[64] = {};
  ColorIndexArray prev = {};
  prev.rgba.a = 255;
  ColorIndexArray px = prev;
  uint8_t run = 0;
  while (idx < (int)imageBinary.size()) {
    px.rgba.r = imageBinary[idx + 0];
    px.rgba.g = imageBinary[idx + 1];
    px.rgba.b = imageBinary[idx + 2];
    if (channels == 4)
      px.rgba.a = imageBinary[idx + 3];

    if (px.v == prev.v) {
      run++;
      if (run == 62) {
        output.put(0xc0 | (run - 1));
        run = 0;
      }
    } else {
      if (run > 0) { // RUN
        output.put(0xc0 | (run - 1));
        run = 0;
      }
      if (px.rgba.a == prev.rgba.a) {
        signed char vr = px.rgba.r - prev.rgba.r;
        signed char vg = px.rgba.g - prev.rgba.g;
        signed char vb = px.rgba.b - prev.rgba.b;

        signed char vg_r = vr - vg;
        signed char vg_b = vb - vg;

        if (vr > -3 && vr < 2 && vg > -3 && vg < 2 && vb > -3 &&
            vb < 2) { // DIFF
          output.put(0x40 | (vr + 2) << 4 | (vg + 2) << 2 | (vb + 2));
        } else if (vg_r > -9 && vg_r < 8 && vg > -33 && vg < 32 && vg_b > -9 &&
                   vg_b < 8) { // LUMA
          output.put(0x80 | (vg + 32));
          output.put((vg_r + 8) << 4 | (vg_b + 8));
        } else { // RGB
          output.put(0xfe);
          output.put(px.rgba.r);
          output.put(px.rgba.g);
          output.put(px.rgba.b);
        }
      } else { // RGBA
        output.put(0xff);
        output.put(px.rgba.r);
        output.put(px.rgba.g);
        output.put(px.rgba.b);
        output.put(px.rgba.a);
      }
    }

    index[getIndex(px.rgba.r, px.rgba.g, px.rgba.b, px.rgba.a)] = px;
    prev = px;
    idx += channels;
  }
  if (run > 0)
    output.put(0xc0 | (run - 1));
}

} // namespace qoi
