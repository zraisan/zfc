cmake_minimum_required(VERSION 3.23)

project(zdc
    VERSION 0.1.0
    DESCRIPTION "Zero-dependency converter: from-scratch image format encoder/decoder"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ZDC_BUILD_EXECUTABLE "Build the zdc CLI executable" ON)

# Core library usable from other C/C++ projects
add_library(zdc_lib
    ${CMAKE_SOURCE_DIR}/src/image/image.cpp
    ${CMAKE_SOURCE_DIR}/src/image/bmp.cpp
    ${CMAKE_SOURCE_DIR}/src/image/ppm.cpp
    ${CMAKE_SOURCE_DIR}/src/image/tga.cpp
    ${CMAKE_SOURCE_DIR}/src/image/png.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/png_deflate.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/png_filter.cpp
)

target_compile_options(zdc_lib PRIVATE -O3)

target_include_directories(zdc_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/image>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/utils>
    $<INSTALL_INTERFACE:include>
)

# Alias so consumers using add_subdirectory can link against zdc::zdc
add_library(zdc::zdc ALIAS zdc_lib)

# CLI executable
if(ZDC_BUILD_EXECUTABLE)
    add_executable(zdc ${CMAKE_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(zdc PRIVATE zdc_lib)
    target_compile_options(zdc PRIVATE -O3)
endif()
