cmake_minimum_required(VERSION 3.23)

project(zfc)

set(CMAKE_CXX_STANDARD 20)

add_executable(zfc)

target_sources(zfc
  PRIVATE
    src/main.cpp
    src/image/image.cpp
    src/image/bmp.cpp
    src/image/ppm.cpp
)


