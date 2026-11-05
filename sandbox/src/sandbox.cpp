#include "Cacao/CodeRegistry.hpp"

#include "rotator.hpp"

#ifdef _MSC_VER
#define CALL __cdecl
#else
#define CALL
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" EXPORT void CALL __CacaoRegisterTypes() {
	Cacao::CodeRegistry::Get().RegisterFactory<Cacao::Component>("rotator", []() { return new sandbox::Rotator(); }, typeid(sandbox::Rotator));
}