#include "cacaoloader.hpp"

const Cacao::ClientIdentity Cacao::Loader::SelfIdentify() {
	return {.id = "net.rl86.CacaoSandbox", .displayName = "Cacao Engine Sandbox"};
}

void Cacao::Loader::LaunchHook() {}
void Cacao::Loader::TerminateHook() {}