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
			Cacao::WorldManager::GetInstance()->CreateWorld<Cacao::PerspectiveCamera>("Playground");
			Cacao::WorldManager::GetInstance()->SetActiveWorld("Playground");
			Cacao::World& world = Cacao::WorldManager::GetInstance()->GetWorld("Playground");

			std::shared_ptr<SussyScript> ss = std::make_shared<SussyScript>();
			ss->SetActive(true);

			Cacao::ShaderSpec spec;
			shader = new Cacao::Shader("assets/shaders/color.vert.spv", "assets/shaders/color.frag.spv", spec);
			std::future<void> shaderFuture = Cacao::Engine::GetInstance()->GetThreadPool().submit_task([this]() {
				this->shader->Compile();
			});

			Cacao::Model m("assets/models/icosphere.obj");
			mesh = m.ExtractMesh("Icosphere");
			std::future<void> meshFuture = Cacao::Engine::GetInstance()->GetThreadPool().submit_task([this]() {
				this->mesh->Compile();
			});

			std::shared_ptr<Cacao::MeshComponent> mc = std::make_shared<Cacao::MeshComponent>();
			mc->SetActive(true);
			mc->mesh = mesh;
			mc->mat = mat;

			mat->shader = shader;

			meshFuture.wait();
			shaderFuture.wait();

			bob.active = true;
			bob.components.push_back(ss);
			bob.components.push_back(mc);

			world.worldTree.children.push_back(Cacao::TreeItem<Cacao::Entity>(bob));
		}

		void Cleanup() {
			delete mat;
			Cacao::Engine::GetInstance()->GetThreadPool().submit_task([this]() {
				this->shader->Release();
				this->mesh->Release();
			}).wait();
			delete shader;
			delete mesh;
			delete this;
		}
	private:
		static PlaygroundApp* instance;
		static bool instanceExists;

		Cacao::Shader* shader;
		Cacao::Material* mat;
		Cacao::Mesh* mesh;

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