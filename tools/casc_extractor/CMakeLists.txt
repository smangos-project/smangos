add_executable(casc_extractor Main.cpp)
target_link_libraries(casc_extractor PRIVATE casc)
target_compile_features(casc_extractor PRIVATE cxx_std_20)
target_compile_definitions(casc_extractor PRIVATE CASC_EXTRACTOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
