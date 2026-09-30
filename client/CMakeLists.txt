cmake_minimum_required(VERSION 3.20)
project(classic_client_mod CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only pieces shared by the loader and the library.
add_library(client_common INTERFACE)
target_include_directories(client_common INTERFACE common/include)

add_subdirectory(client_library)
add_subdirectory(client_loader)
