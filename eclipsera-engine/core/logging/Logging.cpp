#include "Logging.h"
#include <cstdio>

namespace logging {

void VLog(int level, const char* fmt, va_list ap) {
    char body[2048];
    vsnprintf(body, sizeof(body), fmt, ap);
    TraceLog(level, body);
}

void Log(int level, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VLog(level, fmt, ap);
    va_end(ap);
}

} // namespace logging