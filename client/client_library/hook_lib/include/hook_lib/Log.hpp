#pragma once

#include <cstdarg>

enum class LogLevel
{
    Debug,
    Info,
    Warn,
    Error
};

void OpenLog(const char* path);
void WriteLog(LogLevel level, const char* format, ...);
void WriteLogV(LogLevel level, const char* format, va_list args);
void FlushLog();
void CloseLog();

#define HOOK_LOG_DEBUG(...) ::WriteLog(::LogLevel::Debug, __VA_ARGS__)
#define HOOK_LOG_INFO(...) ::WriteLog(::LogLevel::Info, __VA_ARGS__)
#define HOOK_LOG_WARN(...) ::WriteLog(::LogLevel::Warn, __VA_ARGS__)
#define HOOK_LOG_ERROR(...) ::WriteLog(::LogLevel::Error, __VA_ARGS__)
