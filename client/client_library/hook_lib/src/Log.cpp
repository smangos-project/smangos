#include "hook_lib/Log.hpp"

#include <array>
#include <cstdio>
#include <mutex>
#include <share.h>
#include <string_view>
#include <windows.h>

FILE* LogFile = nullptr;
std::mutex LogMutex;

std::string_view Tag(LogLevel level)
{
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }

    return "INFO";
}

void EnsureParentDirectory(const char* path)
{
    if (!path)
        return;

    std::array<char, MAX_PATH> directory = {};
    strncpy_s(directory.data(), directory.size(), path, _TRUNCATE);

    for (char* cursor = directory.data() + strnlen(directory.data(), directory.size()); cursor != directory.data();
         --cursor) {
        if (*cursor == '\\' || *cursor == '/') {
            *cursor = '\0';
            CreateDirectoryA(directory.data(), nullptr);
            return;
        }
    }
}

void OpenLog(const char* path)
{
    std::lock_guard<std::mutex> lock(LogMutex);
    if (LogFile)
        return;

    EnsureParentDirectory(path);

    // _fsopen rather than fopen_s: the latter opens with sharing denied, which makes the log
    // unreadable while the client is running - exactly when it is worth reading.
    // Unbuffered: this log's main job is explaining crashes and hangs, and a buffered tail is
    // exactly the part that gets lost when the client dies. An accurate log beats a fast one.
    LogFile = _fsopen(path, "w", _SH_DENYWR);
    if (LogFile)
        setvbuf(LogFile, nullptr, _IONBF, 0);
}

void WriteLog(LogLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteLogV(level, format, args);
    va_end(args);
}

void WriteLogV(LogLevel level, const char* format, va_list args)
{
    std::array<char, 1024> body = {};
    vsnprintf(body.data(), body.size(), format, args);

    SYSTEMTIME time = {};
    GetLocalTime(&time);

    std::array<char, 1200> line = {};
    snprintf(line.data(),
             line.size(),
             "[%02u:%02u:%02u] %-5.*s %s\n",
             time.wHour,
             time.wMinute,
             time.wSecond,
             static_cast<int>(Tag(level).size()),
             Tag(level).data(),
             body.data());

    std::lock_guard<std::mutex> lock(LogMutex);
    OutputDebugStringA(line.data());

    if (!LogFile)
        return;

    fputs(line.data(), LogFile);
    if (level >= LogLevel::Warn)
        fflush(LogFile);
}

void FlushLog()
{
    std::lock_guard<std::mutex> lock(LogMutex);
    if (LogFile)
        fflush(LogFile);
}

void CloseLog()
{
    std::lock_guard<std::mutex> lock(LogMutex);
    if (!LogFile)
        return;

    fflush(LogFile);
    fclose(LogFile);
    LogFile = nullptr;
}
