cmake_minimum_required(VERSION 3.23)

project(zfc)

set(CMAKE_CXX_STANDARD 20)

add_executable(zfc)

target_compile_options(zfc PRIVATE -O3)

target_sources(zfc
  PRIVATE
    src/main.cpp
    src/image/image.cpp
    src/image/bmp.cpp
    src/image/ppm.cpp
    src/image/tga.cpp
)


