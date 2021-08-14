cmake_minimum_required(VERSION 3.20)

project(tycart
  VERSION 0.1
  HOMEPAGE_URL https://github.com/tudasc/TyCart
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(
  APPEND
  CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

include(ToolchainOpts)

add_format_target(
  format-tycart-sources "Formats project source files"
  TARGETS lib/pass/*.cpp
          lib/pass/*.h
          lib/runtime/*.cpp
          lib/runtime/*.h
)

add_subdirectory(lib)
add_subdirectory(scripts)

enable_testing()
add_subdirectory(test)
