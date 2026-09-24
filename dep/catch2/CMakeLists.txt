# Catch2, vendored as the amalgamated single header/source pair (see VERSION).
# Upstream: https://github.com/catchorg/Catch2

add_library(Catch2 STATIC extras/catch_amalgamated.cpp)
target_include_directories(Catch2 SYSTEM PUBLIC extras)
target_compile_features(Catch2 PUBLIC cxx_std_20)

# A second build of the same source with CATCH_AMALGAMATED_CUSTOM_MAIN defined, so a target that
# needs its own main() (tests-conformance's --target/--config/--strict handling) can link Catch2's
# machinery without also getting the amalgamated file's built-in one and colliding with it. Only
# tests-conformance (ENABLE_REALBOTS) needs this -- gated the same way so turning RealBots off
# doesn't pay for compiling the amalgamated source a second time for nothing.
if (ENABLE_REALBOTS)
  add_library(Catch2NoMain STATIC extras/catch_amalgamated.cpp)
  target_include_directories(Catch2NoMain SYSTEM PUBLIC extras)
  target_compile_features(Catch2NoMain PUBLIC cxx_std_20)
  target_compile_definitions(Catch2NoMain PUBLIC CATCH_AMALGAMATED_CUSTOM_MAIN)
endif()
