add_executable(tests-realbots
  PlaceholderTest.cpp
)

target_link_libraries(tests-realbots PRIVATE Catch2)
set_target_properties(tests-realbots PROPERTIES FOLDER "Tests")

catch_discover_tests(tests-realbots TEST_PREFIX "realbots: " PROPERTIES LABELS "unit")
