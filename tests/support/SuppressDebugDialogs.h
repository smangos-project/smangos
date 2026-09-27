#ifndef TESTS_SUPPORT_SUPPRESSDEBUGDIALOGS_H
#define TESTS_SUPPORT_SUPPRESSDEBUGDIALOGS_H

#ifdef _WIN32
#include <windows.h>
#ifdef _DEBUG
#include <crtdbg.h>
#include <cstdlib>
#endif
#endif

// Routes the Windows CRT's blocking "Debug Error!" assertion/error dialogs, and Windows Error
// Reporting's crash dialog, to stderr instead: an assertion or crash during an unattended test
// run then fails with readable output in the ctest log, instead of hanging on a dialog nobody is
// there to dismiss. Call once, as early as possible in a test binary's startup. A no-op on
// anything but a Windows Debug build.
inline void SuppressDebugDialogs()
{
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#ifdef _DEBUG
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#endif
}

#endif
