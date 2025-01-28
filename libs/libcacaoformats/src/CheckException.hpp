#pragma once

#include <exception>
#include <string>
#include <stdexcept>

inline void CheckException(bool cond, std::string msg) {
	if(!cond) throw std::runtime_error(msg);
}