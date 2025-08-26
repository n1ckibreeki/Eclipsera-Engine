#pragma once
#include <cstdarg>
#include <raylib.h> // expose log level constants

// Forward declaration for raylib TraceLog
extern "C" void TraceLog(int logLevel, const char* text, ...);

namespace logging {
    void VLog(int level, const char* fmt, va_list ap);
    void Log(int level, const char* fmt, ...);
}

// Macros
#define LOGI(...) logging::Log(LOG_INFO, __VA_ARGS__)
#define LOGW(...) logging::Log(LOG_WARNING, __VA_ARGS__)
#define LOGE(...) logging::Log(LOG_ERROR, __VA_ARGS__)