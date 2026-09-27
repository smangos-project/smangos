#include "SuppressDebugDialogs.h"

// shared/Log.cpp expects the host binary to name its own log file, the way
// mangosd's and bnetserver's Main.cpp each do.
char const* g_mainLogFileName = "tests-shared.log";

namespace
{
struct EarlySuppressDebugDialogs { EarlySuppressDebugDialogs() { SuppressDebugDialogs(); } } g_earlySuppressDebugDialogs;
}
