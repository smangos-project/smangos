# Standalone entry point (used for the mingw-w64 cross build on Linux, see README.md); when
# included from the root project it only adds the targets.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.15)
    project(ClassicClient C CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CLIENT_DEP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../dep/include)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    set(CLIENT_DEP_INCLUDE ${CMAKE_SOURCE_DIR}/dep/include)
endif()

add_library(client_common INTERFACE)
target_include_directories(client_common INTERFACE client_common/include)

add_subdirectory(client_library)
add_subdirectory(client_loader)
