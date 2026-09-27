add_executable(tests-database-tool
  VersionTests.cpp
  World/DoltLiteDatabaseTests.cpp
)

target_link_libraries(tests-database-tool PRIVATE database_tool_lib Catch2)
doltlite_copy_runtime(tests-database-tool)

set_target_properties(tests-database-tool PROPERTIES FOLDER "Tests")

include(${CMAKE_SOURCE_DIR}/dep/catch2/extras/Catch.cmake)
catch_discover_tests(tests-database-tool TEST_PREFIX "database_tool: " PROPERTIES LABELS "unit")
