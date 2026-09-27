add_library(database_tool_lib STATIC
  Version.cpp
)

target_include_directories(database_tool_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(database_tool_lib PUBLIC cxx_std_20)

add_executable(database_tool Main.cpp)
target_link_libraries(database_tool PRIVATE database_tool_lib)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
