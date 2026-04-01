#include "qoi.hpp"

enum class EncodingMethod : uint8_t {
  QOI_OP_RGB = 0,
  QOI_OP_RGBA = 1,
  QOI_OP_INDEX = 2,
  QOI_OP_DIFF = 3,
  QOI_OP_LUMA = 4,
  QOI_OP_RUN = 5
};

EncodingMethod get_encoding_method(uint8_t byte) {
  if (byte == 254)
    return EncodingMethod::QOI_OP_RGB;
  else if (byte == 255)
    return EncodingMethod::QOI_OP_RGBA;
  else if ((byte >> 6) == 0)
    return EncodingMethod::QOI_OP_INDEX;
  else if ((byte >> 6) == 1)
    return EncodingMethod::QOI_OP_DIFF;
  else if ((byte >> 6) == 3)
    return EncodingMethod::QOI_OP_RUN;
  else
    return EncodingMethod::QOI_OP_LUMA;
}


qoi::FileHeader qoi::read_header(const std::vector<unsigned char> &binary) {
  qoi::FileHeader header;
  header.width =
      (binary[4] << 24) | (binary[5] << 16) | (binary[6] << 8) | binary[7];
  header.height =
      (binary[8] << 24) | (binary[9] << 16) | (binary[10] << 8) | binary[11];
  header.channels = binary[12];
  header.colorspace = binary[13];
  header.offset = 14;
  return header;
}

std::vector<unsigned char>
qoi::decode(std::vector<unsigned char> &image_binary) {
  qoi::FileHeader header = qoi::read_header(image_binary);
  std::vector<unsigned char> image_data;

  int idx = header.offset;
  qoi::ColorIndexArray index[64] = {};
  qoi::ColorIndexArray px = {};
  px.rgba.a = 255;
  uint8_t run = 0;
  while (idx < image_binary.size() - header.offset) {
    if (run > 0) {
      run--;
    } else {
      EncodingMethod emethod = get_encoding_method(image_binary[idx]);
      switch (emethod) {
      case EncodingMethod::QOI_OP_RGB:
        idx++;
        px.rgba.r = image_binary[idx++];
        px.rgba.g = image_binary[idx++];
        px.rgba.b = image_binary[idx++];
        break;
      case EncodingMethod::QOI_OP_RGBA:
        idx++;
        px.rgba.r = image_binary[idx++];
        px.rgba.g = image_binary[idx++];
        px.rgba.b = image_binary[idx++];
        px.rgba.a = image_binary[idx++];
        break;
      case EncodingMethod::QOI_OP_INDEX:
        px = index[image_binary[idx] & 0x3f];
        idx++;
        break;
      case EncodingMethod::QOI_OP_DIFF:
        px.rgba.r += ((image_binary[idx] >> 4) & 3) - 2;
        px.rgba.g += ((image_binary[idx] >> 2) & 3) - 2;
        px.rgba.b += ((image_binary[idx]) & 3) - 2;
        idx++;
        break;
      case EncodingMethod::QOI_OP_LUMA: {
        int8_t dg = (image_binary[idx] & 0x3f) - 32;
        idx++;
        px.rgba.r += ((image_binary[idx] >> 4) & 0x0f) - 8 + dg;
        px.rgba.g += dg;
        px.rgba.b += (image_binary[idx] & 0x0f) - 8 + dg;
        idx++;
        break;
      }
      case EncodingMethod::QOI_OP_RUN:
        run = (image_binary[idx] & 0x3f);
        idx++;
        break;
      }
      index[qoi::get_index(px.rgba.r, px.rgba.g, px.rgba.b, px.rgba.a)] = px;
    }
    image_data.push_back(px.rgba.r);
    image_data.push_back(px.rgba.g);
    image_data.push_back(px.rgba.b);
    if (header.channels == 4)
      image_data.push_back(px.rgba.a);
  }
  return image_data;
}
