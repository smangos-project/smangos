add_executable(tests-db-schema-setup
  DbdBuildTests.cpp
  DbdFileTests.cpp
)

target_link_libraries(tests-db-schema-setup PRIVATE db_schema_setup Catch2)
target_compile_definitions(tests-db-schema-setup PRIVATE
  DB_SCHEMA_SETUP_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

set_target_properties(tests-db-schema-setup PROPERTIES FOLDER "Tests")

include(${CMAKE_SOURCE_DIR}/dep/catch2/extras/Catch.cmake)
catch_discover_tests(tests-db-schema-setup TEST_PREFIX "db_schema_setup: " PROPERTIES LABELS "unit")
