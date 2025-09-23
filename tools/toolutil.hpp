#pragma once

#include <iostream>
#include <sstream>

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

#define ERROR(...) \
	std::cerr << "\x1b[0m\x1b[1;91mERROR: \x1b[0m" << __VA_ARGS__ << std::endl;

inline std::stringstream lastMsg;

#define CVLOG_NONL(...)                    \
	VLOG_NONL("\x1b[2K\r" << __VA_ARGS__); \
	lastMsg.str("");                       \
	lastMsg << __VA_ARGS__;
#define CVLOG(...) VLOG("\x1b[2K\r" << lastMsg.str() << __VA_ARGS__)
#define CVLOG_SINGLE(...) VLOG("\x1b[2K\r" << __VA_ARGS__)

#define CompileCheck(condition, ...) \
	if(!(condition)) {               \
		std::stringstream s;         \
		s << __VA_ARGS__;            \
		return {false, s.str()};     \
	}