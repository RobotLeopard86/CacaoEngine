#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>// IWYU pragma: keep

//This contains ABI info that we can verify to ensure that the code is even compatible
struct ABIHandshakeInfo {
	uint32_t pointerSz;	 //sizeof(void*)
	uint32_t sizetSz;	 //sizeof(size_t)
	uint32_t maxAlign;	 //alignof(max_align_t)
	const char* compiler;//"gcc", "clang", "msvc", etc.
	uint32_t compilerVer;//version number (e.g., 1300 for MSVC 19.30)
	uint32_t cppStd;	 //__cplusplus macro value (e.g., 201703, 202002)
	const char* stlLib;	 //"libstdc++", "libc++", "msvc"
	uint32_t stlVer;	 //lib version macro if available
	bool dbg;			 //true if debug build (heuristic, MSVC-sensitive)
};

//This contains the test functions we'll use to verify that everything works fine
struct ABIHandshakeTestFuncs {
	//This ensures that string data can be safely preserved
	std::string (*stringRoundtrip)(const std::string&);

	//This ensures that exceptions thrown from the client can be caught
	void (*clientThrow)();

	//This ensures that exceptions thrown from the engine can be caught
	bool (*engineThrow)(void (*)());

	//This ensures that the client can safely consume smart pointers from the engine and that memory is handled correctly
	bool (*clientConsumePtr)(std::shared_ptr<unsigned int>, unsigned int);

	//This ensures that the engine can safely consume smart pointers from the client and that memory is handled correctly
	std::shared_ptr<unsigned int> (*engineConsumePtr)();

	//This ensures that the client can safely invoke callbacks from the engine
	int (*clientConsumeCallback)(std::function<int(int)>, int);

	//This ensures that the engine can safely invoke callbacks from the client
	std::function<int(int)> (*engineConsumeCallback)();
};