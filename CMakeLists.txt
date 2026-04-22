cmake_minimum_required(VERSION 3.23)

project(zfc)

set(CMAKE_CXX_STANDARD 20)

add_executable(zfc)

target_compile_options(zfc PRIVATE -O3)

 target_include_directories(zfc PRIVATE                                                                                                                                                                                                                                                                    
    ${CMAKE_SOURCE_DIR}/src                                                                                                                                                                                                                                                                                 
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/image
  )

target_sources(zfc
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/image/image.cpp
    ${CMAKE_SOURCE_DIR}/src/image/bmp.cpp
    ${CMAKE_SOURCE_DIR}/src/image/ppm.cpp
    ${CMAKE_SOURCE_DIR}/src/image/tga.cpp
    ${CMAKE_SOURCE_DIR}/src/image/png.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/deflate.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/png_filter.cpp
)


