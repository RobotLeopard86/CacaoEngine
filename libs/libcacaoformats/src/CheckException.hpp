#pragma once

#include <exception>
#include <string>

inline void CheckException(bool cond, std::string msg) {
    if(!cond) throw std::runtime_error(msg);
}