find_package(CURL REQUIRED)

add_executable(casc_extractor Main.cpp Cdn.cpp)
target_include_directories(casc_extractor PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(casc_extractor PRIVATE casc ${CURL_LIBRARY})
target_compile_features(casc_extractor PRIVATE cxx_std_20)
target_compile_definitions(casc_extractor PRIVATE CASC_EXTRACTOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
