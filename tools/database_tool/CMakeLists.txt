add_library(database_tool_lib STATIC
  Version.cpp
  World/DoltLiteDatabase.cpp
  Sql/MysqlLexer.cpp
  Sql/SqlStatementSplitter.cpp
  Sql/MysqlDdlTranslator.cpp
  Migrations/MigrationFile.cpp
  Migrations/MigrationState.cpp
)

target_include_directories(database_tool_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(database_tool_lib PUBLIC cxx_std_20)
target_link_libraries(database_tool_lib PUBLIC doltlite)

add_executable(database_tool Main.cpp)
target_link_libraries(database_tool PRIVATE database_tool_lib)
doltlite_copy_runtime(database_tool)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
