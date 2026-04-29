#include "jpg.hpp"
#include <iostream>
#include <vector>

jpg::FileHeader jpg::read_header(const std::vector<unsigned char> &binary) {
    jpg::FileHeader header;
    header.offset =
        ((binary[4] << 8) | binary[5]) + 4;  // BE length + 4 bytes for SOI + APP0 marker

    // Find SOF marker (0xFFC0 - 0xFFC9, but typically SOF0)
    int idx = 2;  // Start after SOI marker
    while (idx < binary.size() - 9) {
        if (binary[idx] == 0xFF) {
            uint8_t marker = binary[idx + 1];

            // Check if it's a SOF marker
            if ((marker >= 0xC0 && marker <= 0xC3) || (marker >= 0xC5 && marker <= 0xC7) ||
                (marker >= 0xC9 && marker <= 0xCB)) {
                // SOF marker found
                header.bits_per_pixel = binary[idx + 4];                   // Precision (8 or 12)
                header.height = (binary[idx + 5] << 8) | binary[idx + 6];  // BE
                header.width = (binary[idx + 7] << 8) | binary[idx + 8];   // BE
                header.channels = binary[idx + 9];                         // Components

                break;
            }

            // Skip to next marker
            uint16_t length = (binary[idx + 2] << 8) | binary[idx + 3];
            idx += length + 2;
        } else {
            idx++;
        }
    }

    return header;
}

std::vector<unsigned char> jpg::decode(std::vector<unsigned char> &binary) {
    std::vector<unsigned char> decoded_image;
    try {
        // SOI marker
        if (binary[0] != 0xFF || binary[1] != 0xD8)
            throw -1;

        // APP0 marker (JFIF header)
        if (binary[2] != 0xFF || binary[3] != 0xE0)
            throw -1;

        jpg::FileHeader header = jpg::read_header(binary);
        int idx = header.offset;
        std::vector<std::vector<uint16_t>> quant_tables;
        quant_tables.resize(4);  // Up to 4 tables (0-3)

        while (idx + 1 < (int)binary.size()) {
            // Every segment should starts with FF MARKER
            if (binary[idx] != 0xFF) {
                idx++;
                continue;
            }
            uint8_t marker = binary[idx + 1];

            // Standalone markers (no length, no payload): SOI, EOI, RST0..RST7, TEM
            if (marker == 0xD8 || (marker >= 0xD0 && marker <= 0xD7) || marker == 0x01) {
                idx += 2;
                continue;
            }
            if (marker == 0xD9)
                break;

            // All other markers have FF MM LL LL [payload]
            int length = (binary[idx + 2] << 8) | binary[idx + 3];

            if (marker == 0xDB) {
                // DQT: quantization tables. TODO: store for later dequantization.
                int qt_idx = idx + 4;
                while (qt_idx < length + qt_idx - 2) {
                    bool pq = binary[qt_idx] >> 4;       // Precision (0 for 8-bit, 1 for 16-bit)
                    uint8_t tq = binary[qt_idx] & 0x0F;  // Table identifier (0-3)
                    quant_tables[tq].resize(64);
                    qt_idx++;
                    for (int i = 0; i < 64; i++) {
                        quant_tables[tq][i] = binary[qt_idx + i];
                    }
                    qt_idx += 64;
                }

            } else if ((marker >= 0xC0 && marker <= 0xC3) || (marker >= 0xC5 && marker <= 0xC7) ||
                       (marker >= 0xC9 && marker <= 0xCB)) {
                // SOF: frame info already extracted by read_header.
            } else if (marker == 0xC4) {
                // DHT: huffman tables. TODO: build decoding tables.
            } else if (marker == 0xDA) {
                // SOS: entropy-coded scan begins. TODO: decode coefficients,
                // dequantize, zig-zag unscan, IDCT, YCbCr to RGB.
                break;
            }

            idx += 2 + length;  // marker (2 bytes) + length value (which includes itself)
        }
    } catch (int e) {
        std::cout << "Error: This is not a valid JPG format";
    }
    return decoded_image;
}
