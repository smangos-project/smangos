add_executable(tests-database-tool
  VersionTests.cpp
  World/DoltLiteDatabaseTests.cpp
  Sql/MysqlLexerTests.cpp
  Sql/SqlStatementSplitterTests.cpp
  Sql/MysqlDdlTranslatorTests.cpp
  Migrations/MigrationFileTests.cpp
  Migrations/MigrationStateTests.cpp
)

target_link_libraries(tests-database-tool PRIVATE database_tool_lib Catch2)
doltlite_copy_runtime(tests-database-tool)
target_compile_definitions(tests-database-tool PRIVATE
  DATABASE_TOOL_TEST_MIGRATIONS_DIR="${CMAKE_SOURCE_DIR}/sql/migrations")

set_target_properties(tests-database-tool PROPERTIES FOLDER "Tests")

include(${CMAKE_SOURCE_DIR}/dep/catch2/extras/Catch.cmake)
catch_discover_tests(tests-database-tool TEST_PREFIX "database_tool: " PROPERTIES LABELS "unit")
