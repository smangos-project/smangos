add_library(db_schema_setup STATIC
  DbdBuild.cpp
  DbdFile.cpp
  DbdResolve.cpp
  DbdDefinitions.cpp
  DbcGenerator.cpp
)

target_include_directories(db_schema_setup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(db_schema_setup PUBLIC cxx_std_20)

include(FetchContent)

# Pinned to the newest WoWDBDefs commit whose definitions cover our client build
# (1.15.6.58797, dataBuild 1.15.6.58725). Bumping the pin means updating the SHA and its hash
# and nothing else. SOURCE_SUBDIR names a directory that doesn't exist in the archive, so CMake
# never tries to add the WoWDBDefs repo's own (unrelated) code as a subproject.
set(DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT e989e99e6f5f97c57b2e138d4d28b16066b4ee9c)

FetchContent_Declare(wowdbdefs
    URL                        https://github.com/wowdev/WoWDBDefs/archive/${DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT}.tar.gz
    URL_HASH                   SHA256=969347da770cfd56237fd4002efea916fa90cd54318c76475294fe777f36c4c3
    SOURCE_SUBDIR              _wowdbdefs_no_build_
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(wowdbdefs)

target_compile_definitions(db_schema_setup PUBLIC
  DB_SCHEMA_SETUP_DEFINITIONS_DIR="${wowdbdefs_SOURCE_DIR}/definitions"
  DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT="${DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT}")

add_executable(db_schema_setup_check Check.cpp)
target_link_libraries(db_schema_setup_check PRIVATE db_schema_setup)

add_executable(db_schema_setup_generate Generate.cpp)
target_link_libraries(db_schema_setup_generate PRIVATE db_schema_setup)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
