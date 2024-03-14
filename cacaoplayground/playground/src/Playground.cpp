#include "Cacao.hpp"

#include <chrono>

extern "C" {
	class SussyScript : public Cacao::Script {
	public:
		void OnActivate() override {
			Cacao::Logging::ClientLog("I'm awake!");
		}
		void OnDeactivate() override {
			Cacao::Logging::ClientLog("I'm asleep!");
		}
		void OnTick(double timestep) override {
			Cacao::Logging::ClientLog(std::string("Okay so ello from tick ") + std::to_string(count) + ", where it's been " + std::to_string(timestep * 1000) + " milliseconds.");
			count++;
		}
	private:
		int count;
	};

	static Cacao::PerspectiveCamera* cam;

	void _CacaoLaunch() {
		Cacao::Window::GetInstance()->SetTitle("Cacao Playground");

		Cacao::WorldManager::GetInstance()->CreateWorld<Cacao::PerspectiveCamera>("Playground");
		Cacao::WorldManager::GetInstance()->SetActiveWorld("Playground");
		Cacao::World& world = Cacao::WorldManager::GetInstance()->GetWorld("Playground");

		std::shared_ptr<SussyScript> ss = std::make_shared<SussyScript>();
		ss->SetActive(true);

		Cacao::Entity bob;
		bob.active = true;
		bob.components.push_back(ss);

		world.worldTree.children.push_back(Cacao::TreeItem<Cacao::Entity>(bob));

		Cacao::Engine::GetInstance()->cfg.targetDynTPS = 5;
	}

	void _CacaoExiting() {
		delete cam;
	}
}