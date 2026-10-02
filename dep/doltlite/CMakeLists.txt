# DoltLite: SQLite fork with Git-style version control, used for the world
# database (see .local/plan/wms/database_tool/plan.md). Version control is
# plain SQL on an ordinary sqlite3 connection (dolt_commit, dolt_log, ...),
# so this target is just the sqlite3 C API plus a couple of doltlite_*
# extras, backed by DoltLite's native engine instead of stock SQLite.
#
# Pinned to DoltHub.Doltlite 0.50.12 (native engines built from
# dolthub/doltlite@83693647b6c7c0e4a898d1b96fbe0f23913b9156, SQLITE_VERSION
# 3.54.0). Bumping the pin means updating the two URL_HASH/EXPECTED_HASH
# values below, the commit in the sqlite.h.in URL, and the three SQLITE_*
# substitution values (from that commit's VERSION/manifest/manifest.uuid).
# See README.md for why the dependency is fetched and linked this way.

include(FetchContent)

if (WIN32)
  set(DOLTLITE_RID "win-x64")
elseif (APPLE)
  set(DOLTLITE_RID "osx-arm64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(DOLTLITE_RID "linux-arm64")
else()
  set(DOLTLITE_RID "linux-x64")
endif()

# NuGet has no plain "give me the zip" URL: DOWNLOAD_NAME forces a .zip
# extension so FetchContent's extractor recognizes the .nupkg as one.
FetchContent_Declare(doltlite_native
  URL                        https://www.nuget.org/api/v2/package/DoltHub.Doltlite/0.50.12
  URL_HASH                   SHA256=27fe0e2d790c0715e6cb0c352b3209ca3506e7e4c9f2cf17ddf2efb7780d4381
  DOWNLOAD_NAME              doltlite.0.50.12.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(doltlite_native)

if (WIN32)
  set(DOLTLITE_LIBRARY_FILE "${doltlite_native_SOURCE_DIR}/runtimes/${DOLTLITE_RID}/native/doltlite.dll")
elseif (APPLE)
  set(DOLTLITE_LIBRARY_FILE "${doltlite_native_SOURCE_DIR}/runtimes/${DOLTLITE_RID}/native/libdoltlite.dylib")
else()
  set(DOLTLITE_LIBRARY_FILE "${doltlite_native_SOURCE_DIR}/runtimes/${DOLTLITE_RID}/native/libdoltlite.so")
endif()

if (NOT EXISTS "${DOLTLITE_LIBRARY_FILE}")
  message(FATAL_ERROR "doltlite: expected native library not found at ${DOLTLITE_LIBRARY_FILE}")
endif()

# The NuGet package carries only the .NET wrapper and the native binaries,
# not sqlite3.h. Rebuild it from the pinned commit's src/sqlite.h.in the way
# DoltLite's own mksqlite3h.tcl does: substitute the version markers and
# close the header guard mksqlite3h.tcl normally appends.
set(DOLTLITE_SQLITE_H_IN "${CMAKE_CURRENT_BINARY_DIR}/sqlite.h.in")
if (NOT EXISTS "${DOLTLITE_SQLITE_H_IN}")
  file(DOWNLOAD
    "https://raw.githubusercontent.com/dolthub/doltlite/83693647b6c7c0e4a898d1b96fbe0f23913b9156/src/sqlite.h.in"
    "${DOLTLITE_SQLITE_H_IN}"
    EXPECTED_HASH SHA256=6939eb53e2f0f49f215262968212e2898dedbb6a5f4f3d69377d163eb9dea7c2
  )
endif()

set(DOLTLITE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(DOLTLITE_SQLITE_H "${DOLTLITE_INCLUDE_DIR}/sqlite3.h")
if (NOT EXISTS "${DOLTLITE_SQLITE_H}")
  file(READ "${DOLTLITE_SQLITE_H_IN}" _sqlite_h_content)
  string(REPLACE "--VERS--"           "3.54.0"  _sqlite_h_content "${_sqlite_h_content}")
  string(REPLACE "--VERSION-NUMBER--" "3054000" _sqlite_h_content "${_sqlite_h_content}")
  string(REPLACE "--SOURCE-ID--"
    "2026-08-31 20:43:10 6f73383647fdf579dca72e15cc8a9627be72fd524e8e843e7d62dfc9253e20d2"
    _sqlite_h_content "${_sqlite_h_content}")
  # Branch/tags/build-datetime are diagnostic-only strings, not compiled
  # against, so a placeholder is fine here (unlike the three values above).
  string(REPLACE "--SCM-BRANCH--"     "trunk" _sqlite_h_content "${_sqlite_h_content}")
  string(REPLACE "--SCM-TAGS--"       ""      _sqlite_h_content "${_sqlite_h_content}")
  string(REPLACE "--SCM-DATETIME--"   "2026-08-31 20:43:10" _sqlite_h_content "${_sqlite_h_content}")
  string(APPEND _sqlite_h_content "\n#endif /* SQLITE3_H */\n")
  file(MAKE_DIRECTORY "${DOLTLITE_INCLUDE_DIR}")
  file(WRITE "${DOLTLITE_SQLITE_H}" "${_sqlite_h_content}")
endif()

add_library(doltlite SHARED IMPORTED GLOBAL)
set_target_properties(doltlite PROPERTIES
  IMPORTED_LOCATION             "${DOLTLITE_LIBRARY_FILE}"
  INTERFACE_INCLUDE_DIRECTORIES "${DOLTLITE_INCLUDE_DIR}"
)

set(DOLTLITE_RUNTIME_DLLS)
if (UNIX AND NOT APPLE)
  # The library's SONAME is libdoltlite.so.0 but the package ships it as
  # libdoltlite.so, so the loader needs a copy under the SONAME next to the
  # executable (found through the $ORIGIN rpath set in doltlite_copy_runtime).
  set(DOLTLITE_RUNTIME_DLLS "${DOLTLITE_LIBRARY_FILE}")
endif()
if (WIN32)
  # An MSVC-ABI linker -- MSVC's own, clang-cl, or plain clang++ targeting
  # the MSVC ABI and linked with lld-link -- can't link a MinGW-built DLL
  # without an import library, and DoltLite doesn't ship one for win-x64:
  # generate it from the DLL's own export table. `MSVC` alone misses the
  # clang++/lld-link case (compiler frontend GNU, linker frontend MSVC), so
  # both are checked.
  if (MSVC OR CMAKE_CXX_COMPILER_LINKER_FRONTEND_VARIANT STREQUAL "MSVC")
    get_filename_component(_msvc_tools_dir "${CMAKE_LINKER}" DIRECTORY)

    # Real MSVC (or clang-cl next to a Visual Studio install) gives us
    # dumpbin/lib; an LLVM-only toolchain has no Visual Studio tools next to
    # its linker, so fall back to llvm-readobj/llvm-lib, which read/write
    # the same export table and COFF import library format.
    find_program(DOLTLITE_DUMPBIN dumpbin HINTS "${_msvc_tools_dir}")
    find_program(DOLTLITE_LIB_TOOL lib    HINTS "${_msvc_tools_dir}")
    set(DOLTLITE_EXPORTS_TOOL_KIND "dumpbin")
    set(DOLTLITE_LIB_TOOL_KIND "lib")

    if (NOT DOLTLITE_DUMPBIN OR NOT DOLTLITE_LIB_TOOL)
      find_program(DOLTLITE_DUMPBIN llvm-readobj HINTS "${_msvc_tools_dir}")
      find_program(DOLTLITE_LIB_TOOL llvm-lib    HINTS "${_msvc_tools_dir}")
      set(DOLTLITE_EXPORTS_TOOL_KIND "llvm-readobj")
      set(DOLTLITE_LIB_TOOL_KIND "llvm-lib")
    endif()

    if (NOT DOLTLITE_DUMPBIN OR NOT DOLTLITE_LIB_TOOL)
      message(FATAL_ERROR
        "doltlite: neither dumpbin/lib nor llvm-readobj/llvm-lib found next to the linker (${_msvc_tools_dir})")
    endif()

    set(DOLTLITE_IMPORT_LIB "${CMAKE_CURRENT_BINARY_DIR}/doltlite.lib")
    if (NOT EXISTS "${DOLTLITE_IMPORT_LIB}")
      execute_process(
        COMMAND ${CMAKE_COMMAND}
          -DEXPORTS_TOOL=${DOLTLITE_DUMPBIN}
          -DEXPORTS_TOOL_KIND=${DOLTLITE_EXPORTS_TOOL_KIND}
          -DLIBTOOL=${DOLTLITE_LIB_TOOL}
          -DLIBTOOL_KIND=${DOLTLITE_LIB_TOOL_KIND}
          -DDLL=${DOLTLITE_LIBRARY_FILE}
          -DDLL_NAME=doltlite.dll
          -DOUT_DEF=${CMAKE_CURRENT_BINARY_DIR}/doltlite.def
          -DOUT_LIB=${DOLTLITE_IMPORT_LIB}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateImportLib.cmake
        RESULT_VARIABLE _gen_result
      )
      if (NOT _gen_result EQUAL 0)
        message(FATAL_ERROR "doltlite: failed to generate the import library")
      endif()
    endif()
    set_target_properties(doltlite PROPERTIES IMPORTED_IMPLIB "${DOLTLITE_IMPORT_LIB}")
  endif()

  # DoltLite is built with MinGW; these are its only dependencies beyond
  # what every Windows install already has (bcrypt, crypt32, ws2_32, ...).
  list(APPEND DOLTLITE_RUNTIME_DLLS
    "${DOLTLITE_LIBRARY_FILE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/win-x64/libgcc_s_seh-1.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/win-x64/libwinpthread-1.dll"
  )
endif()

# CACHE INTERNAL, not a plain variable: doltlite_copy_runtime() below is
# called from tools/database_tool, which is outside dep/doltlite's own
# directory scope and wouldn't otherwise see this list.
set(DOLTLITE_RUNTIME_DLLS "${DOLTLITE_RUNTIME_DLLS}" CACHE INTERNAL "")

# Copies doltlite's runtime DLLs next to TARGET's output. Every executable
# that links doltlite, directly or through a static library, must call this.
function(doltlite_copy_runtime TARGET)
  if(UNIX AND NOT APPLE)
    set_property(TARGET ${TARGET} APPEND PROPERTY INSTALL_RPATH "\$ORIGIN")
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DOLTLITE_RUNTIME_DLLS}" "$<TARGET_FILE_DIR:${TARGET}>/libdoltlite.so.0"
    )
    return()
  endif()
  foreach(_dll ${DOLTLITE_RUNTIME_DLLS})
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "$<TARGET_FILE_DIR:${TARGET}>"
    )
  endforeach()
endfunction()

add_subdirectory(smoketest)
