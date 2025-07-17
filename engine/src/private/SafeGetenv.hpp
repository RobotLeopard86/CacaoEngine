#pragma once

#include <string>
#include <cstdlib>

#define safe_getenv(v) []() { const char* p = std::getenv(v); if(p == nullptr) return std::string(""); else return std::string(p); }()