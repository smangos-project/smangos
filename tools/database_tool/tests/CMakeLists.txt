add_executable(tests-database-tool
  VersionTests.cpp
  World/DoltLiteDatabaseTests.cpp
  World/WorldArchiveTests.cpp
  World/WorldCompareTests.cpp
  World/VersionCollapseTests.cpp
  Sql/MysqlLexerTests.cpp
  Sql/SqlStatementSplitterTests.cpp
  Sql/MysqlDdlTranslatorTests.cpp
  Sql/MysqlInsertParserTests.cpp
  Sql/MysqlDumpImportTests.cpp
  Sql/MysqlDumpImportRealDumpTests.cpp
  Sql/MigrationPorterTests.cpp
  Migrations/MigrationFileTests.cpp
  Migrations/MigrationStateTests.cpp
  Migrations/MigrationRunnerTests.cpp
  ToolConfigTests.cpp
  Commands/CreateMysqlDatabaseTests.cpp
  Commands/CreateWorldDatabaseTests.cpp
  Commands/PortMigrationFileTests.cpp
  Dbc/DbcExportTests.cpp
  Dbc/DbcImportTests.cpp
  Dbc/DbcSchemaTests.cpp
  Dbc/WdbcTests.cpp
  Dbc/WdbcRealFilesTests.cpp
  Dbc/Wdc5ReaderTests.cpp
  Dbc/Wdc5WriterTests.cpp
  Dbc/Wdc5RealFilesTests.cpp
  Spec/SpecFileTests.cpp
  Spec/SpecInventoryTests.cpp
  Spec/SourceScanTests.cpp
  Spec/WireHintTests.cpp
  Spec/CounterpartTests.cpp
  Spec/ShapeTests.cpp
  Spec/EvidenceTests.cpp
  Spec/GapAnalysisTests.cpp
)

target_link_libraries(tests-database-tool PRIVATE database_tool_lib Catch2)
doltlite_copy_runtime(tests-database-tool)
database_tool_copy_mysql_runtime(tests-database-tool)
target_compile_definitions(tests-database-tool PRIVATE
  DATABASE_TOOL_TEST_MIGRATIONS_DIR="${CMAKE_SOURCE_DIR}/sql/migrations"
  DATABASE_TOOL_TEST_SQL_DIR="${CMAKE_SOURCE_DIR}/sql"
  DATABASE_TOOL_TEST_WORLD_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/World/fixtures"
  DATABASE_TOOL_TEST_REPO_ROOT_DIR="${CMAKE_SOURCE_DIR}")

set_target_properties(tests-database-tool PROPERTIES FOLDER "Tests")

include(${CMAKE_SOURCE_DIR}/dep/catch2/extras/Catch.cmake)
catch_discover_tests(tests-database-tool TEST_PREFIX "database_tool: " PROPERTIES LABELS "unit")
