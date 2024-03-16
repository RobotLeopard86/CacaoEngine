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

	class PlaygroundApp {
	public:
		static PlaygroundApp* GetInstance() {
			//Do we have an instance yet?
			if(!instanceExists || instance == NULL){
				//Create instance
				instance = new PlaygroundApp();
				instanceExists = true;
			}

			return instance;
		}

		void Launch() {
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

			Cacao::Engine::GetInstance()->cfg.targetDynTPS = 30;
		}

		void Cleanup() {
			//delete sky;
			//skyTex->Release();
			//delete skyTex;
			delete this;
		}
	private:
		static PlaygroundApp* instance;
		static bool instanceExists;

		Cacao::Cubemap* skyTex;
		Cacao::Skybox* sky;

		Cacao::Entity bob;
	};

	PlaygroundApp* PlaygroundApp::instance = nullptr;
	bool PlaygroundApp::instanceExists = false;

	void _CacaoLaunch() {
		PlaygroundApp::GetInstance()->Launch();
	}

	void _CacaoExiting() {
		PlaygroundApp::GetInstance()->Cleanup();
	}
}