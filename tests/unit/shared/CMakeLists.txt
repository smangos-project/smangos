add_executable(tests-shared
  PlaceholderTest.cpp
)

target_link_libraries(tests-shared PRIVATE Catch2)
set_target_properties(tests-shared PROPERTIES FOLDER "Tests")

catch_discover_tests(tests-shared TEST_PREFIX "shared: " PROPERTIES LABELS "unit")
