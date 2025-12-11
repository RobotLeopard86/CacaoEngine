#include "Runtime.hpp"
#include "abi/ABI.hpp"

void Runtime::SetupEngine() {
	//Initialize engine
	Cacao::Engine::Get().CoreInit(icfg);
	Cacao::Engine::Get().GfxInit();

	//TODO: Configure resource loading
}

ABIHandshakeInfo GetRuntimeABIInfo() {
	ABIHandshakeInfo abi {};

	abi.pointerSz = sizeof(void*);
	abi.sizetSz = sizeof(std::size_t);
	abi.maxAlign = alignof(std::max_align_t);

	//Compiler + version
#if defined(__clang__)
	abi.compiler = "clang";
	abi.compilerVer = (__clang_major__ * 10000) +
					  (__clang_minor__ * 100) +
					  (__clang_patchlevel__);
#elif defined(_MSC_VER)
	abi.compiler = "msvc";
	abi.compilerVer = _MSC_VER;
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

void Runtime::LoadGame() {
	//Load binary
	gameBinary = std::make_unique<dynalo::library>(cacaospec.binary);

	//Perform ABI checks
	ABIHandshakeInfo gameABI = gameBinary->get_function<ABIHandshakeInfo()>("__CacaoAbiInfoHandshake")();
	ABIHandshakeInfo ourABI = GetRuntimeABIInfo();
#define binpanic(msg) panic(msg, "This usually means the game binary is not compiled with the correct settings to match the runtime.")
	if(gameABI.dbg != ourABI.dbg) binpanic("Debug mode mismatch");
	if(gameABI.pointerSz != ourABI.pointerSz) binpanic("Pointer size mismatch");
	if(gameABI.sizetSz != ourABI.sizetSz) binpanic("size_t size mismatch");
	if(gameABI.maxAlign != ourABI.maxAlign) binpanic("Max alignment mismatch");
	if(gameABI.compilerVer < ourABI.compilerVer) binpanic("Game binary uses outdated compiler version");
	if(gameABI.cppStd < ourABI.cppStd) binpanic("Game binary uses outdated C++ standard");
	if(std::string(ourABI.stlLib).compare(gameABI.stlLib) != 0) binpanic("STL provider library mismatch");
	if(gameABI.stlVer != ourABI.stlVer) binpanic("STL version mismatch");
	std::string ourCompiler(ourABI.compiler);
	std::string gameCompiler(gameABI.compiler);
#ifdef _WIN32
	if(ourCompiler.compare(gameCompiler) != 0) {
		if(ourCompiler.compare("clang") == 0) {
			if(gameCompiler.compare("msvc") != 0) binpanic("Compiler mismatch");
		} else if(ourCompiler.compare("msvc") == 0) {
			if(gameCompiler.compare("clang") != 0) binpanic("Compiler mismatch");
		} else if(ourCompiler.compare("gcc") == 0) {
			binpanic("Compiler mismatch");
		} else {
			binpanic("Unknown runtime compiler");
		}
	}
#endif
#undef binpanic
}

void Runtime::DestroyGfxObjects() {
	//TODO
}

void Runtime::Cleanup() {
	//Unload binary
	gameBinary.reset();
}