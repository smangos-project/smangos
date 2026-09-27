#include "Version.h"

const char* DatabaseToolVersion()
{
    return "0.1.0";
}

// shared/Log.cpp expects the executable to supply this (see src/mangosd/Main.cpp);
// defined once here, in the static library, so both database_tool and
// tests-database-tool get it without a duplicate-definition conflict.
char const* g_mainLogFileName = "database_tool.log";
