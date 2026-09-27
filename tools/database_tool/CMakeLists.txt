add_library(database_tool_lib STATIC
  Version.cpp
  ToolConfig.cpp
  World/DoltLiteDatabase.cpp
  Sql/MysqlLexer.cpp
  Sql/SqlStatementSplitter.cpp
  Sql/MysqlDdlTranslator.cpp
  Migrations/MigrationFile.cpp
  Migrations/MigrationState.cpp
  Migrations/MigrationRunner.cpp
  Commands/CreateMysqlDatabase.cpp
)

target_include_directories(database_tool_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(database_tool_lib PUBLIC cxx_std_20)
target_link_libraries(database_tool_lib PUBLIC doltlite shared framework ${MYSQL_LIBRARY})

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
doltlite_copy_runtime(database_tool)
database_tool_copy_mysql_runtime(database_tool)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
