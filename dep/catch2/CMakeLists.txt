# Catch2, vendored as the amalgamated single header/source pair (see VERSION).
# Upstream: https://github.com/catchorg/Catch2

add_library(Catch2 STATIC extras/catch_amalgamated.cpp)
target_include_directories(Catch2 SYSTEM PUBLIC extras)
target_compile_features(Catch2 PUBLIC cxx_std_20)
