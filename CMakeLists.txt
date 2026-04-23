cmake_minimum_required(VERSION 3.23)

project(zfc
    VERSION 0.1.0
    DESCRIPTION "From-scratch image format encoder/decoder"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ZFC_BUILD_EXECUTABLE "Build the zfc CLI executable" ON)

# Core library — usable from other C/C++ projects
add_library(zfc_lib
    ${CMAKE_SOURCE_DIR}/src/image/image.cpp
    ${CMAKE_SOURCE_DIR}/src/image/bmp.cpp
    ${CMAKE_SOURCE_DIR}/src/image/ppm.cpp
    ${CMAKE_SOURCE_DIR}/src/image/tga.cpp
    ${CMAKE_SOURCE_DIR}/src/image/png.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/deflate.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/png_filter.cpp
)

target_compile_options(zfc_lib PRIVATE -O3)

target_include_directories(zfc_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/image>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/utils>
    $<INSTALL_INTERFACE:include>
)

# Alias so consumers using add_subdirectory can link against zfc::zfc
add_library(zfc::zfc ALIAS zfc_lib)

# CLI executable
if(ZFC_BUILD_EXECUTABLE)
    add_executable(zfc ${CMAKE_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(zfc PRIVATE zfc_lib)
    target_compile_options(zfc PRIVATE -O3)
endif()
