# Builds an MSVC import library for doltlite.dll from its export table.
# DoltLite is built with MinGW and ships no .lib, so MSVC can't link it
# directly. Run in script mode with -D DUMPBIN=... LIBTOOL=... DLL=...
# DLL_NAME=... OUT_DEF=... OUT_LIB=....

if (NOT EXISTS "${DLL}")
  message(FATAL_ERROR "doltlite: DLL not found: ${DLL}")
endif()

execute_process(
  COMMAND "${DUMPBIN}" /EXPORTS "${DLL}"
  OUTPUT_VARIABLE _dumpbin_output
  RESULT_VARIABLE _dumpbin_result
)
if (NOT _dumpbin_result EQUAL 0)
  message(FATAL_ERROR "doltlite: dumpbin /EXPORTS failed on ${DLL}")
endif()

string(REPLACE "\n" ";" _lines "${_dumpbin_output}")
set(_names)
foreach(_line ${_lines})
  if (_line MATCHES "^ *[0-9]+ +[0-9A-Fa-f]+ +[0-9A-Fa-f]+ +([A-Za-z_][A-Za-z0-9_]*) *\r?$")
    list(APPEND _names "${CMAKE_MATCH_1}")
  endif()
endforeach()

list(LENGTH _names _name_count)
if (_name_count EQUAL 0)
  message(FATAL_ERROR "doltlite: found no exported symbols in ${DLL}")
endif()

set(_def_content "LIBRARY ${DLL_NAME}\nEXPORTS\n")
foreach(_name ${_names})
  string(APPEND _def_content "${_name}\n")
endforeach()
file(WRITE "${OUT_DEF}" "${_def_content}")

execute_process(
  COMMAND "${LIBTOOL}" /NOLOGO "/DEF:${OUT_DEF}" "/OUT:${OUT_LIB}" /MACHINE:X64 "/NAME:${DLL_NAME}"
  RESULT_VARIABLE _lib_result
  OUTPUT_QUIET
)
if (NOT _lib_result EQUAL 0)
  message(FATAL_ERROR "doltlite: lib.exe failed to build an import library from ${OUT_DEF}")
endif()

message(STATUS "doltlite: generated import library ${OUT_LIB} (${_name_count} symbols)")
