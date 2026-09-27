#ifndef LZMASDK_EXTRACT7Z_H
#define LZMASDK_EXTRACT7Z_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Extracts the first non-directory file of a plain (non-multivolume,
// non-encrypted) .7z archive to outPath, overwriting it if it exists.
// Returns 0 on success. On failure returns non-zero and, if errorOut is
// non-null, writes a NUL-terminated message (truncated to fit) to it.
// Kept free of the SDK's own headers so callers never pull in <Windows.h>.
int Lzmasdk_Extract7z(char const* archivePath, char const* outPath, char* errorOut, size_t errorOutSize);

#ifdef __cplusplus
}
#endif

#endif
