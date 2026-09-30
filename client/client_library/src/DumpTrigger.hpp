#pragma once

// On-demand minidump of the running client, for looking at a hang (e.g. a
// black screen with no crash). External debuggers are refused by the client,
// even non-invasive ones, but a dump written from inside the process is not.
//
// Create a file named `custom.dumpnow` next to WowClassic.exe; within about a
// second the client writes `WowClassic.custom.dmp` there (with thread info
// and stacks) and deletes the trigger. Read it with
// `cdb -z WowClassic.custom.dmp -c "~*k; q"`.
void StartDumpTrigger();
