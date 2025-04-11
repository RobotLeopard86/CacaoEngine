#pragma once

#include <iostream>

enum class OutputLevel {
	Silent,
	Normal,
	Verbose
};

inline OutputLevel outputLvl;

#define VLOG(...) \
	if(outputLvl == OutputLevel::Verbose) std::cout << __VA_ARGS__ << std::endl;
#define VLOG_NONL(...) \
	if(outputLvl == OutputLevel::Verbose) std::cout << __VA_ARGS__ << std::flush;