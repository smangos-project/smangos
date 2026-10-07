add_library(database_tool_lib STATIC
  Version.cpp
  ToolConfig.cpp
  World/DoltLiteDatabase.cpp
  World/WorldArchive.cpp
  World/ZipExtract.cpp
  World/WorldCompare.cpp
  World/VersionCollapse.cpp
  Reference/ReferenceBuild.cpp
  Sql/MysqlLexer.cpp
  Sql/SqlStatementSplitter.cpp
  Sql/MysqlDdlTranslator.cpp
  Sql/MysqlInsertParser.cpp
  Sql/MysqlDumpImport.cpp
  Sql/MigrationPorter.cpp
  Migrations/MigrationFile.cpp
  Migrations/MigrationState.cpp
  Migrations/MigrationRunner.cpp
  Commands/CreateMysqlDatabase.cpp
  Commands/CreateWorldDatabase.cpp
  Commands/GenerateWms.cpp
  Fdid/FileDataIds.cpp
  Commands/PortMigrationFile.cpp
  Dbc/DbcExport.cpp
  Dbc/DbcHotfixes.cpp
  Dbc/DbcImport.cpp
  Dbc/DbcSchema.cpp
  Dbc/InputManifest.cpp
  Dbc/Wdbc.cpp
  Dbc/Wdc5Reader.cpp
  Dbc/Wdc5Writer.cpp
  Spec/SpecFile.cpp
  Spec/SpecInventory.cpp
  Spec/SourceScan.cpp
  Spec/WireHint.cpp
  Spec/ColumnMatch.cpp
  Spec/Counterpart.cpp
  Spec/Shape.cpp
  Spec/Evidence.cpp
  Spec/GapAnalysis.cpp
  Spec/SpecCheck.cpp
  Spec/SpecRender.cpp
  Verify/RowCompare.cpp
  Verify/Verify.cpp
  generated/DbcTables.g.cpp
  generated/VanillaDbcTables.g.cpp
)

target_include_directories(database_tool_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(database_tool_lib PUBLIC cxx_std_20)
# OPENSSL_LIBRARIES: shared's own SHA256/BigNumber need libcrypto, which
# (like MYSQL_LIBRARY above) mangosd/bnetserver only pull in at their own
# executable's link step, in a release/debug split this tool doesn't need.
# db_schema_setup: Spec/GapAnalysis.cpp (4.5.1) reuses its WoWDBDefs resolver to diff the
# vanilla and client layouts -- see contrib/db_schema_setup/CMakeLists.txt's own pin.
target_link_libraries(database_tool_lib PUBLIC doltlite lzmasdk shared framework db_schema_setup ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${OPENSSL_EXTRA_LIBRARIES})

# mangosd/bnetserver only get these via `cmake --install` (see the root
# CMakeLists.txt install() rules); a standalone tool needs its own copy next
# to its own output to run at all, installed or not.
function(database_tool_copy_mysql_runtime TARGET)
  if (WIN32)
    foreach(_dll libeay32.dll libmySQL.dll)
      add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/dep/windows/lib/${DEP_ARCH}_release/${_dll}"
          "$<TARGET_FILE_DIR:${TARGET}>"
      )
    endforeach()
  endif()
endfunction()

add_executable(database_tool Main.cpp)
target_link_libraries(database_tool PRIVATE database_tool_lib)
# The CLI's own default paths (.world/, sql/migrations); see plan.md decision
# 1 -- there is no command-line override, tests call the library directly
# with temp paths instead.
target_compile_definitions(database_tool PRIVATE DATABASE_TOOL_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
doltlite_copy_runtime(database_tool)
database_tool_copy_mysql_runtime(database_tool)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
