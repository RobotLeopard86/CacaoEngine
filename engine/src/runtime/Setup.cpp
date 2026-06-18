#include "Cacao/Log.hpp"
#include "Cacao/WorldManager.hpp"
#include "Runtime.hpp"
#include "abi/ABI.hpp"
#include <exception>
#include <memory>
#include <stdexcept>

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
	Cacao::Logger::Runtime() << "Welcome to \"" << rt.cacaospec.meta.title << "\" (" << rt.cacaospec.meta.pkgId << ")!";
	Cacao::Logger::Runtime(Cacao::Logger::Level::Trace) << "Loading game binary...";

	//Load binary
	gameBinary = std::make_unique<dynalo::library>(cacaospec.binary);

	//Perform ABI checks
	Cacao::Logger::Runtime(Cacao::Logger::Level::Trace) << "Performing ABI compatibility checks...";
	ABIHandshakeInfo gameABI = gameBinary->get_function<ABIHandshakeInfo()>("__CacaoAbiInfoHandshake")();
	ABIHandshakeInfo ourABI = GetRuntimeABIInfo();
#define binpanic(msg) panic(msg, "This usually means the game binary is not compiled with the correct settings to match the runtime")
	if(gameABI.dbg != ourABI.dbg) binpanic("Debug mode mismatch");
	if(gameABI.pointerSz != ourABI.pointerSz) binpanic("Pointer size mismatch");
	if(gameABI.sizetSz != ourABI.sizetSz) binpanic("size_t size mismatch");
	if(gameABI.maxAlign != ourABI.maxAlign) binpanic("Max alignment mismatch");
	if(gameABI.compilerVer < ourABI.compilerVer) binpanic("Game binary uses outdated compiler version");
	if(gameABI.cppStd < ourABI.cppStd) binpanic("Game binary uses outdated C++ standard");
	if(std::string(ourABI.stlLib).compare(gameABI.stlLib) != 0) binpanic("STL provider library mismatch");
	if(gameABI.stlVer != ourABI.stlVer) binpanic("STL version mismatch");
#ifdef _WIN32
	std::string ourCompiler(ourABI.compiler);
	std::string gameCompiler(gameABI.compiler);
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
	ABIHandshakeTestFuncs funcs = gameBinary->get_function<ABIHandshakeTestFuncs()>("__CacaoAbiFuncsHandshake")();
	if(funcs.stringRoundtrip("¿Cómo estás?").compare("¡Estoy bien!") != 0) binpanic("String exchange failed");
	try {
		funcs.clientThrow();
	} catch(const std::exception& e) {
		if(std::string(e.what()).compare("exceptions ok") != 0) binpanic("Runtime cannot receive game exceptions");
	}
	if(!funcs.engineThrow([]() {
		   throw std::runtime_error("exceptions ok");
	   })) binpanic("Game binary cannot receive engine exceptions");
	std::shared_ptr<unsigned int> testPtr = std::make_shared<unsigned int>(67);
	if(!funcs.clientConsumePtr(testPtr, 67)) binpanic("Game binary cannot receive engine smart pointers");
	if(*funcs.engineConsumePtr() != 37) binpanic("Engine cannot receive game binary smart pointers");
	if(funcs.clientConsumeCallback([](int val) { return val * 2; }, 913) != ((913 * 2) + 1)) binpanic("Game binary cannot receive engine callbacks");
	if(funcs.engineConsumeCallback()(72) != 74) binpanic("Engine cannot receive game binary callbacks!");
#undef binpanic

	Cacao::Logger::Runtime() << "Game package successfully loaded!";
	Cacao::Logger::Runtime(Cacao::Logger::Level::Trace) << "Preparing to run...";

	//Load and activate initial world
	Cacao::WorldManager::Get().SetActiveWorld(rt.cacaospec.startupWorld);
}

void Runtime::DestroyGfxObjects() {
	//TODO
}

void Runtime::Cleanup() {
	//Unload binary
	Cacao::Logger::Runtime(Cacao::Logger::Level::Trace) << "Unloading game binary...";
	gameBinary.reset();
}