#include "Cacao/Engine.hpp"

int main() {
	Cacao::Engine::InitConfig icfg = {};
	Cacao::Engine::Get().CoreInit(icfg);
}