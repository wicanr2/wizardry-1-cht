cmake_minimum_required(VERSION 3.16)
project(wizardry_cht CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# vcpkg ships proper CMake Config packages; prefer those (CONFIG mode), but
# fall back to plain find_library on systems where SDL2 is installed via
# distro package (Ubuntu apt etc.) without Config files.

find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
  find_package(SDL2 REQUIRED)
endif()

find_package(SDL2_ttf CONFIG QUIET)
if(NOT SDL2_ttf_FOUND AND NOT TARGET SDL2_ttf::SDL2_ttf)
  find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf REQUIRED)
  find_path(SDL2_TTF_INCLUDE_DIR NAMES SDL_ttf.h PATH_SUFFIXES SDL2 REQUIRED)
endif()

find_package(SDL2_image CONFIG QUIET)
if(NOT SDL2_image_FOUND AND NOT TARGET SDL2_image::SDL2_image)
  find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
  find_path(SDL2_IMAGE_INCLUDE_DIR NAMES SDL_image.h PATH_SUFFIXES SDL2)
endif()

find_package(SDL2_mixer CONFIG QUIET)
if(NOT SDL2_mixer_FOUND AND NOT TARGET SDL2_mixer::SDL2_mixer)
  find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer)
  find_path(SDL2_MIXER_INCLUDE_DIR NAMES SDL_mixer.h PATH_SUFFIXES SDL2)
endif()

find_package(nlohmann_json CONFIG QUIET)

add_subdirectory(src)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
