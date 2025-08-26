#include "FileSystem.h"
#include "core/logging/Logging.h"
#include <fstream>
#include <sstream>

namespace fsys {

std::string ReadFileToString(const std::string& path) {
    LOGI("ReadFileToString: opening '%s'", path.c_str());
    std::ifstream file(path);
    if (!file.is_open()) {
        LOGE("ReadFileToString: failed to open '%s'", path.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto s = buffer.str();
    LOGI("ReadFileToString: read %zu bytes from '%s'", s.size(), path.c_str());
    return s;
}

} // namespace fsys