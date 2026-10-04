
# Generic boolean options
option(BUILD_FOR_HOST_CPU             "Build specifically for the host CPU via `-march=native` (might not run on different machines)" ON)
option(BUILD_WARNINGS_AS_ERROR        "Treat compiler warnings as errors (/WX on MSVC, -Werror on GCC/Clang)"                         OFF)
option(USE_SCRIPTS                    "Build scripts"                                                                                 ON)
option(BUILD_EXTRACTORS               "Build extractors"                                                                              OFF)
option(BUILD_REALMMERGE               "Build helper tool for merging character databases"                                             OFF)
option(ENABLE_MAILSENDER              "Enables support for sending emails via sendgrid.com (requires libcurl)"                        OFF)
option(ENABLE_CPPTRACE                "Enables cpptrace stack tracing; can be disabled if using other tools like GDB"                 ON)
option(ENABLE_REALBOTS                "Build the RealBots client bot framework"                                                       ON)
option(BUILD_TESTING                  "Build unit and conformance test suites"                                                        OFF)
option(BUILD_REALBOT_RUNNER           "Build the RealBots console host (contrib/realbotrunner)"                                       OFF)
option(ENABLE_ASAN                    "Build the whole tree with AddressSanitizer (MSVC /fsanitize=address, clang -fsanitize=address)" OFF)
option(BUILD_DB_SCHEMA_SETUP          "Build the WoWDBDefs schema reader (contrib/db_schema_setup)"                                    OFF)
option(BUILD_CLIENT                   "Build the client loader and library (Windows only, client/)"                                    ON)
option(BUILD_DATABASE_TOOL            "Build the database setup tool (tools/database_tool)"                                            OFF)
option(BUILD_MAP_EXTRACTOR            "Build the map/vmap/mmap extractor (tools/map_extractor)"                                         OFF)
option(BUILD_CASC_EXTRACTOR           "Build the CASC client extractor (tools/casc_extractor)"                                          OFF)

if (BUILD_MAP_EXTRACTOR AND NOT BUILD_DATABASE_TOOL)
  set(BUILD_DATABASE_TOOL ON CACHE BOOL "Build the database setup tool (tools/database_tool)" FORCE)
endif()

if (BUILD_REALBOT_RUNNER AND NOT ENABLE_REALBOTS)
  message(FATAL_ERROR "BUILD_REALBOT_RUNNER requires ENABLE_REALBOTS to be ON")
endif()

if(USE_EXTRACTORS)
  message(WARNING "USE_EXTRACTORS is deprecated, please use BUILD_EXTRACTORS instead")
  set(BUILD_EXTRACTORS ON CACHE BOOL "Build extractors" FORCE)
endif()

if(USE_REALMMERGE)
  message(WARNING "USE_REALMMERGE is deprecated, please use BUILD_REALMMERGE instead")
  set(BUILD_REALMMERGE ON CACHE BOOL "Build helper tool for merging character databases" FORCE)
endif()

if(UNIX)
  option(DEBUG_SYMBOLS "Include Debug Symbols" ON)
endif()

# Precompile headers stuff
find_package(PCHSupport)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16")
  option(USE_PCH "Use precompiled headers" ON)
endif()

# TODO: I have no idea why `USE_PCH_OLD`, is needed if there is `USE_PCH`
if(PCHSupport_FOUND AND (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"))
  if(MSVC)
    option(USE_PCH_OLD "Use old precompiled headers implementation" ON)
  else()
    option(USE_PCH_OLD "Use old precompiled headers implementation" OFF)
  endif()
else()
  unset(USE_PCH_OLD CACHE)
endif()

if((${CMAKE_VERSION} VERSION_LESS "3.16") OR USE_PCH_OLD)
  if(PCHSupport_FOUND)
    if(WIN32)
      option(USE_PCH "Use precompiled headers" ON)
    else()
      option(USE_PCH "Use precompiled headers" OFF)
    endif()
  endif()
else()
  option(USE_PCH "Use precompiled headers" ON)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

message("")
message(STATUS
  "This script builds the vMaNGOS server.
  Options that can be used in order to configure the process:
    PREFIX                    Path where the server should be installed to
    CMAKE_INSTALL_PREFIX      Path where the server should be installed to
    USE_PCH                   Use precompiled headers
    USE_PCH_OLD               Use precompiled headers
    BUILD_FOR_HOST_CPU        Build specifically for the host CPU via `-march=native` (might not run on different machines)
    BUILD_WARNINGS_AS_ERROR   Treat compiler warnings as errors (/WX on MSVC, -Werror on GCC/Clang)
    USE_SCRIPTS               Build scripts
    BUILD_EXTRACTORS          Build extractors
    BUILD_REALMMERGE          Build helper tool for merging character databases
    ENABLE_MAILSENDER         Enables support for sending emails via sendgrid.com (requires libcurl)
    ENABLE_CPPTRACE           Enables cpptrace stack tracing; can be disabled if using other tools like GDB
    ENABLE_REALBOTS           Build the RealBots client bot framework
    BUILD_TESTING             Build unit and conformance test suites
    BUILD_REALBOT_RUNNER      Build the RealBots console host (contrib/realbotrunner)
    ENABLE_ASAN               Build the whole tree with AddressSanitizer
    BUILD_DB_SCHEMA_SETUP     Build the WoWDBDefs schema reader (contrib/db_schema_setup)
    BUILD_DATABASE_TOOL       Build the database setup tool (tools/database_tool)
    BUILD_MAP_EXTRACTOR       Build the map/vmap/mmap extractor (tools/map_extractor)
    BUILD_CASC_EXTRACTOR      Build the CASC client extractor (tools/casc_extractor)


  To set an option simply type -D<OPTION>=<VALUE> after 'cmake <srcs>'.
  Also, you can specify the generator with -G. see 'cmake --help' for more details
  For example:
    Build server with default settings:
    $ mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../opt/vmangos ..
")
