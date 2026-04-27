<div align="center">

# zfc

**From-Scratch Image Format Encoder and Decoder**

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg?logo=cplusplus)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.23%2B-064F8C.svg?logo=cmake)](https://cmake.org/)

</div>

---

zfc is a C++20 library and command-line tool that reads, writes, and converts image formats from scratch. No libpng, no stb_image, **no third-party dependencies at all**. Every format is parsed and produced byte-by-byte directly from its specification, using only the C++ standard library. The project doubles as a ground-up implementation of the compression primitives behind modern image formats, including LZ77, Huffman coding, DEFLATE, zlib, CRC-32, and Adler-32.

## Features

- **Zero Dependencies**: Only the C++ standard library is linked. No libpng, no libz, no stb_image. Fewer dependencies means a smaller supply chain, a smaller attack surface, and no CVE triage for third-party image parsers
- **Byte-Level Format I/O**: Reads and writes BMP, PPM, TGA, QOI, and PNG directly from and to raw bytes
- **RFC 1951 DEFLATE**: Full encoder and decoder with dynamic and fixed Huffman blocks, stored blocks, LZ77 back-references, and length-limited canonical codes
- **Zlib Container**: 2-byte header and Adler-32 checksum compatible with libpng, zlib, and Pillow
- **PNG Chunk Framing**: CRC-32 validation and proper IHDR, PLTE, IDAT, and IEND emission with color-type-aware rules
- **Adaptive Filtering**: All five PNG filter types (None, Sub, Up, Average, Paeth) with a per-row MSAD heuristic for choosing the best filter
- **Library or CLI**: Invoke through the `zfc` executable, or link as a CMake sub-project to use the same primitives from your own C++ code

## Installation

### From Source

```bash
git clone https://github.com/zraisan/zfc.git
cd zfc

cmake -B build
cmake --build build

./build/zfc
```

### Requirements

- A C++20 compiler (GCC 11 or later, Clang 13 or later)
- CMake 3.23 or later

## Usage

```bash
zfc image <input> <output>
```

Input and output formats are inferred from the file extension. Examples:

```bash
zfc image photo.bmp photo.png
zfc image shot.ppm shot.bmp
zfc image render.bmp render.ppm
```

## Supported Formats

### BMP

- Reader and writer for uncompressed BI_RGB bitmaps
- Handles 4-byte row padding and both bottom-up and top-down orientations

### PPM

- Binary (P6) reader and writer
- 8-bit-per-channel RGB pixel data

### TGA

- Uncompressed truecolor reader and writer
- Handles both top-down and bottom-up orientations

### QOI

- Full Quite OK Image reader and writer
- Chunk-based encoding with index, diff, luma, run, and RGB/RGBA tags

### PNG

- Encoder for 8-bit grayscale, grayscale + alpha, truecolor, and truecolor + alpha
- Decoder for zlib-compressed IDAT data using the in-repo DEFLATE implementation
- All five adaptive filter types with MSAD-based per-row selection
- Dynamic Huffman coding with length-limited canonical codes
- Correct CRC-32 on every chunk and Adler-32 on the zlib stream

## Using as a Library

Drop zfc into another CMake project:

```cmake
add_subdirectory(path/to/zfc)
target_link_libraries(my_app PRIVATE zfc::zfc)
```

Skip the CLI build when embedding:

```cmake
set(ZFC_BUILD_EXECUTABLE OFF)
add_subdirectory(path/to/zfc)
```

Encode from C++:

```cpp
#include "image/png.hpp"

png::FileHeader header{width, height, 3, 24, 0};
png::encode(header, image_data, "out.png");
```

C++ only. No C bindings are provided.

## Architecture

zfc is organized around per-format encoder and decoder pairs backed by shared compression primitives:

```
src/
  image/      Format-specific readers and writers (bmp, png, ppm, tga, qoi)
  utils/      Reusable primitives (deflate, huffman, png_filter)
  main.cpp    CLI entry point
```

The DEFLATE implementation in `src/utils/deflate.cpp` powers the PNG encoder and can be reused by future formats that need Huffman coding or LZ77.

## Contributing

Contributions are welcome.

### Adding a New Format

1. Create a pair of files in `src/image/` named with the three-letter format extension (for example `gif.cpp` and `gif.hpp`).
2. Wrap all format code in a namespace with the same three-letter name:

    ```cpp
    namespace gif {
        struct FileHeader { /* width, height, channels, etc. */ };

        FileHeader read_header(const std::vector<unsigned char> &binary);
        std::vector<unsigned char> decode(std::vector<unsigned char> &binary);

        template <typename FH>
        void encode(FH &header,
                    std::vector<unsigned char> &data,
                    std::string output_path);
    }
    ```

3. Wire the new format into `process_image` in `src/image/image.cpp` so the CLI dispatches on the file extension.
4. Add the new `.cpp` to the `zfc_lib` source list in `CMakeLists.txt`.

### Adding Shared Primitives

Reusable infrastructure (compression, filtering, bit I/O, checksums) lives in `src/utils/`. Follow the existing `deflate` and `png_filter` style: a paired header and source file with a namespace matching the filename.

### Style

Keep the code C++20, free of third-party image libraries, and self-contained. Prefer standard-library containers and algorithms over platform-specific APIs.

## License

GNU Affero General Public License v3.0. See [LICENSE](LICENSE) for the full text.
