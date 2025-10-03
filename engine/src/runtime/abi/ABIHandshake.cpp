//This file is not part of the runtime itself; rather it is compiled into games using the runtime to perform ABI verification

#include "ABI.hpp"

#include <cstddef>

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT ABIHandshakeInfo __CacaoAbiInfoHandshake() {
	ABIHandshakeInfo abi {};

	abi.pointerSz = sizeof(void*);
	abi.sizetSz = sizeof(std::size_t);
	abi.maxAlign = alignof(std::max_align_t);

	//Compiler + version
#if defined(_MSC_VER)
	abi.compiler = "msvc";
	abi.compilerVer = _MSC_VER;
#elif defined(__clang__)
	abi.compiler = "clang";
	abi.compilerVer = (__clang_major__ * 10000) +
					  (__clang_minor__ * 100) +
					  (__clang_patchlevel__);
#elif defined(__GNUC__)
	abi.compiler = "gcc";
	abi.compilerVer = (__GNUC__ * 10000) +
					  (__GNUC_MINOR__ * 100) +
					  (__GNUC_PATCHLEVEL__);
#else
	abi.compiler = "unknown";
	abi.compilerVer = 0;
#endif

	//C++ standard
#ifdef __cplusplus
	abi.cppStd = __cplusplus;
#else
	abi.cppStd = 0;
#endif

	//STL detection
#if defined(_LIBCPP_VERSION)
	abi.stlLib = "libc++";
	abi.stlVer = _LIBCPP_VERSION;
#elif defined(__GLIBCXX__)
	abi.stlLib = "libstdc++";
	abi.stlVer = __GLIBCXX__;
#elif defined(_MSVC_STL_VERSION)
	abi.stlLib = "msvc";
	abi.stlVer = _MSVC_STL_VERSION;
#else
	abi.stlLib = "unknown";
	abi.stlVer = 0;
#endif

	//Debug mode detection
#if defined(_MSC_VER)
	abi.dbg = (_ITERATOR_DEBUG_LEVEL > 0);
#elif defined(_GLIBCXX_DEBUG) || defined(_LIBCPP_DEBUG)
	abi.dbg = true;
#elif defined(_DEBUG)
	abi.dbg = true;
#else
	abi.dbg = false;//fallback: assume not debug if no _DEBUG macro present
#endif

	return abi;
}

//Test functions

static std::string stringRoundtrip(const std::string& input) {
	if(input != "¿Cómo estás?")
		return "error";
	return "¡Estoy bien!";
}

static bool engineThrow(void (*thrower)()) {
	try {
		thrower();
	} catch(const std::runtime_error& e) {
		return std::string(e.what()) == "exceptions ok";
	}
	return false;
}

static void clientThrow() {
	throw std::runtime_error("exceptions ok");
}

static std::shared_ptr<unsigned int> engineConsumePtr() {
	return std::make_shared<unsigned int>(42);
}

static bool clientConsumePtr(std::shared_ptr<unsigned int> ptr, unsigned int expected) {
	return ptr && *ptr == expected;
}

static int clientConsumeCallback(std::function<int(int)> func, int arg) {
	if(!func)
		throw std::runtime_error("invalid function");
	return func(arg) + 1;
}

static std::function<int(int)> engineConsumeCallback() {
	return [](int x) { return x + 2; };
}

EXPORT ABIHandshakeTestFuncs __CacaoAbiFuncsHandshake() {
	return {
		.stringRoundtrip = stringRoundtrip,
		.clientThrow = clientThrow,
		.engineThrow = engineThrow,
		.clientConsumePtr = clientConsumePtr,
		.engineConsumePtr = engineConsumePtr,
		.clientConsumeCallback = clientConsumeCallback,
		.engineConsumeCallback = engineConsumeCallback};
}