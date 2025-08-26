#pragma once
#include <string>

namespace fsys {

// Read an entire file into a string
std::string ReadFileToString(const std::string& path);

} // namespace fsys