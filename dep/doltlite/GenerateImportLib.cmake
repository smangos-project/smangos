# Builds an MSVC-style import library for doltlite.dll from its export
# table. DoltLite is built with MinGW and ships no .lib, so an MSVC-ABI
# linker (MSVC's own, clang-cl, or clang++/lld-link) can't link it
# directly. Run in script mode with -D EXPORTS_TOOL=... EXPORTS_TOOL_KIND=
# dumpbin|llvm-readobj LIBTOOL=... LIBTOOL_KIND=lib|llvm-lib DLL=...
# DLL_NAME=... OUT_DEF=... OUT_LIB=....

if (NOT EXISTS "${DLL}")
  message(FATAL_ERROR "doltlite: DLL not found: ${DLL}")
endif()

if (EXPORTS_TOOL_KIND STREQUAL "llvm-readobj")
  execute_process(
    COMMAND "${EXPORTS_TOOL}" --coff-exports "${DLL}"
    OUTPUT_VARIABLE _exports_output
    RESULT_VARIABLE _exports_result
  )
else()
  execute_process(
    COMMAND "${EXPORTS_TOOL}" /EXPORTS "${DLL}"
    OUTPUT_VARIABLE _exports_output
    RESULT_VARIABLE _exports_result
  )
endif()
if (NOT _exports_result EQUAL 0)
  message(FATAL_ERROR "doltlite: ${EXPORTS_TOOL_KIND} failed to read exports from ${DLL}")
endif()

string(REPLACE "\n" ";" _lines "${_exports_output}")
set(_names)
if (EXPORTS_TOOL_KIND STREQUAL "llvm-readobj")
  # llvm-readobj --coff-exports prints one "  Name: <symbol>" line per export.
  foreach(_line ${_lines})
    if (_line MATCHES "^ *Name: +([A-Za-z_][A-Za-z0-9_]*) *\r?$")
      list(APPEND _names "${CMAKE_MATCH_1}")
    endif()
  endforeach()
else()
  # dumpbin /EXPORTS prints "ordinal hint RVA name" per export.
  foreach(_line ${_lines})
    if (_line MATCHES "^ *[0-9]+ +[0-9A-Fa-f]+ +[0-9A-Fa-f]+ +([A-Za-z_][A-Za-z0-9_]*) *\r?$")
      list(APPEND _names "${CMAKE_MATCH_1}")
    endif()
  endforeach()
endif()

list(LENGTH _names _name_count)
if (_name_count EQUAL 0)
  message(FATAL_ERROR "doltlite: found no exported symbols in ${DLL}")
endif()

set(_def_content "LIBRARY ${DLL_NAME}\nEXPORTS\n")
foreach(_name ${_names})
  string(APPEND _def_content "${_name}\n")
endforeach()
file(WRITE "${OUT_DEF}" "${_def_content}")

if (LIBTOOL_KIND STREQUAL "llvm-lib")
  # llvm-lib has no /NAME equivalent; the .def's own LIBRARY line covers it.
  execute_process(
    COMMAND "${LIBTOOL}" "/def:${OUT_DEF}" "/out:${OUT_LIB}" /machine:x64
    RESULT_VARIABLE _lib_result
    OUTPUT_QUIET
  )
else()
  execute_process(
    COMMAND "${LIBTOOL}" /NOLOGO "/DEF:${OUT_DEF}" "/OUT:${OUT_LIB}" /MACHINE:X64 "/NAME:${DLL_NAME}"
    RESULT_VARIABLE _lib_result
    OUTPUT_QUIET
  )
endif()
if (NOT _lib_result EQUAL 0)
  message(FATAL_ERROR "doltlite: ${LIBTOOL_KIND} failed to build an import library from ${OUT_DEF}")
endif()

message(STATUS "doltlite: generated import library ${OUT_LIB} (${_name_count} symbols)")
