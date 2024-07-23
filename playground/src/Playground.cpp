#include "Cacao.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <chrono>

#define ICOSPHERE_COUNT 9
#define ICOSPHERE_RANGE 5

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

class PlaygroundApp {
  public:
	static PlaygroundApp* GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new PlaygroundApp();
			instanceExists = true;
		}

		return instance;
	}

	void Launch();

	void Cleanup() {
		//Release assets
		icoShader->Release();
		icoMesh->Release();
		prisShader->Release();
		prisMesh->Release();
		prisTex->Release();
		sky->Release();

		//Delete world
		Cacao::WorldManager::GetInstance()->RemoveWorld("Playground");

		//Delete instance
		delete this;
	}

	Cacao::AssetHandle<Cacao::Skybox> GetSky() {
		return sky;
	}

	void PlayStopTone();

  private:
	static PlaygroundApp* instance;
	static bool instanceExists;

	Cacao::AssetHandle<Cacao::Skybox> sky;
	Cacao::AssetHandle<Cacao::Sound> bgm;
	Cacao::AssetHandle<Cacao::Sound> stopTone;

	Cacao::AssetHandle<Cacao::Shader> icoShader;
	Cacao::AssetHandle<Cacao::Mesh> icoMesh;
	std::shared_ptr<Cacao::Material> icoMat;
	std::vector<std::shared_ptr<Cacao::Entity>> icospheres;

	std::shared_ptr<Cacao::Entity> cameraManager;
	xg::Guid apGuid;

	Cacao::AssetHandle<Cacao::Shader> prisShader;
	Cacao::AssetHandle<Cacao::Mesh> prisMesh;
	Cacao::AssetHandle<Cacao::Texture2D> prisTex;
	std::shared_ptr<Cacao::Material> prisMat;
	std::shared_ptr<Cacao::Entity> p1, p2;
};

class SussyScript final : public Cacao::Script {
  public:
	void OnActivate() override {
		Cacao::Logging::ClientLog("I'm awake!");
		usedCtrl = false;
	}
	void OnDeactivate() override {
		Cacao::Logging::ClientLog("I'm asleep!");
	}
	void Exit() {
		Cacao::Engine::GetInstance()->GetThreadPool()->enqueue_detach([]() {
			PlaygroundApp::GetInstance()->PlayStopTone();
			Cacao::Engine::GetInstance()->Stop();
		});
	}
	void OnTick(double timestep) override {
		if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_ESCAPE)) Exit();

		if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_LEFT_CONTROL) && !usedCtrl) {
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_F)) {
				Cacao::Window::GetInstance()->SetMode(Cacao::WindowMode::Fullscreen);
				usedCtrl = true;
			} else if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_B)) {
				Cacao::Window::GetInstance()->SetMode(Cacao::WindowMode::Borderless);
				usedCtrl = true;
			} else if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_W)) {
				Cacao::Window::GetInstance()->SetMode(Cacao::WindowMode::Window);
				usedCtrl = true;
			} else if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_X)) {
				Exit();
			}
		} else if(!Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_LEFT_CONTROL)) {
			usedCtrl = false;
			Cacao::World& world = Cacao::WorldManager::GetInstance()->GetActiveWorld();
			Cacao::PerspectiveCamera* cam = static_cast<Cacao::PerspectiveCamera*>(world.cam);
			glm::vec3 camRotChange = glm::vec3(0.0f);
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_J)) {
				camRotChange.y -= 0.5f;
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_K)) {
				camRotChange.y += 0.5f;
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_Y)) {
				camRotChange.x += 0.5f;
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_U)) {
				camRotChange.x -= 0.5f;
			}

			currentRot = cam->GetRotation();
			currentRot += camRotChange;

			if(currentRot.x < -89.99) {
				currentRot.x = -89.99f;
			}
			if(currentRot.x > 89.99) {
				currentRot.x = 89.99f;
			}
			if(currentRot.y < 0) {
				currentRot.y = 360.0f;
			}
			if(currentRot.y > 360) {
				currentRot.y = 0.0f;
			}
			if(currentRot.z < 0) {
				currentRot.z = 360.0f;
			}
			if(currentRot.z > 360) {
				currentRot.z = 0.0f;
			}

			glm::vec3 posChange = glm::vec3(0.0f);
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_W)) {
				posChange += cam->GetFrontVector() * 5.0f * float(timestep);
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_S)) {
				posChange -= cam->GetFrontVector() * 5.0f * float(timestep);
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_D)) {
				posChange += cam->GetRightVector() * 5.0f * float(timestep);
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_A)) {
				posChange -= cam->GetRightVector() * 5.0f * float(timestep);
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_E)) {
				posChange += cam->GetUpVector() * 5.0f * float(timestep);
			}
			if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_Q)) {
				posChange -= cam->GetUpVector() * 5.0f * float(timestep);
			}

			currentPos = cam->GetPosition() + posChange;

			cam->SetRotation(currentRot);
			cam->SetPosition(currentPos);
		}
	}

  private:
	glm::vec3 currentRot, currentPos;
	bool usedCtrl;
};

class PingPong : public Cacao::Script {
  public:
	PingPong(glm::vec3 point1, glm::vec3 point2)
	  : p1(point1), p2(point2), direction(point2 - point1), current(point1), currentDistance(0.0f), totalDistance(glm::length(direction)) {}

	void OnActivate() override {
		forward = true;
		GetOwner().lock()->GetLocalTransform().SetPosition(p1);
	}
	void OnTick(double timestep) override {
		float distanceToInterpolate = timestep;

		if(forward) {
			//Move towards p2
			currentDistance += distanceToInterpolate;
			if(currentDistance >= totalDistance) {
				current = p2;
				forward = false;
				currentDistance = 0.0f;
			} else {
				current = p1 + (direction * (currentDistance / totalDistance));
			}
		} else {
			//Move towards p1
			currentDistance += distanceToInterpolate;
			if(currentDistance >= totalDistance) {
				current = p1;
				forward = true;
				currentDistance = 0.0f;
			} else {
				current = p2 - (direction * (currentDistance / totalDistance));
			}
		}

		//Update the transform value
		GetOwner().lock()->GetLocalTransform().SetPosition(current);
	}

  private:
	glm::vec3 p1, p2, direction, current;
	float currentDistance, totalDistance;
	bool forward;
};

class Spinner : public Cacao::Script {
  public:
	void OnTick(double timestep) {
		glm::vec3 rot = GetOwner().lock()->GetLocalTransform().GetRotation();
		rot.y += 50.0f * timestep;
		while(rot.y >= 360) rot.y -= 360;
		GetOwner().lock()->GetLocalTransform().SetRotation(rot);
	}
};

PlaygroundApp* PlaygroundApp::instance = nullptr;
bool PlaygroundApp::instanceExists = false;

void PlaygroundApp::PlayStopTone() {
	std::shared_ptr<Cacao::AudioPlayer> p = cameraManager->GetComponent<Cacao::AudioPlayer>(apGuid);
	p->Stop();
	p->sound = stopTone;
	p->SetLooping(false);
	p->SetGain(0.8f);
	p->Play();
	std::this_thread::sleep_for(std::chrono::milliseconds(4500));
}

void PlaygroundApp::Launch() {
	//Create a world
	Cacao::WorldManager::GetInstance()->CreateWorld<Cacao::PerspectiveCamera>("Playground");
	Cacao::WorldManager::GetInstance()->SetActiveWorld("Playground");
	Cacao::World& world = Cacao::WorldManager::GetInstance()->GetWorld("Playground");

	//Load assets
	std::future<Cacao::AssetHandle<Cacao::Skybox>> skyFuture = Cacao::AssetManager::GetInstance()->LoadSkybox("assets/sky/sky.cubedef.yml");
	std::future<Cacao::AssetHandle<Cacao::Sound>> bgmFuture = Cacao::AssetManager::GetInstance()->LoadSound("assets/audio/chords.opus");
	std::future<Cacao::AssetHandle<Cacao::Sound>> stFuture = Cacao::AssetManager::GetInstance()->LoadSound("assets/audio/stoptone.mp3");
	std::future<Cacao::AssetHandle<Cacao::Shader>> icoShaderFuture = Cacao::AssetManager::GetInstance()->LoadShader("assets/shaders/ico.shaderdef.yml");
	std::future<Cacao::AssetHandle<Cacao::Mesh>> icoMeshFuture = Cacao::AssetManager::GetInstance()->LoadMesh("assets/models/icosphere.obj:Icosphere");
	std::future<Cacao::AssetHandle<Cacao::Shader>> prisShaderFuture = Cacao::AssetManager::GetInstance()->LoadShader("assets/shaders/prism.shaderdef.yml");
	std::future<Cacao::AssetHandle<Cacao::Mesh>> prisMeshFuture = Cacao::AssetManager::GetInstance()->LoadMesh("assets/models/triprism.obj:Cone");
	std::future<Cacao::AssetHandle<Cacao::Texture2D>> prisTexFuture = Cacao::AssetManager::GetInstance()->LoadTexture2D("assets/tex/prism.png");

	//Get loaded assets
	sky = skyFuture.get();
	bgm = bgmFuture.get();
	stopTone = stFuture.get();
	icoShader = icoShaderFuture.get();
	icoMesh = icoMeshFuture.get();
	prisShader = prisShaderFuture.get();
	prisMesh = prisMeshFuture.get();
	prisTex = prisTexFuture.get();

	//Create materials
	icoMat = std::make_shared<Cacao::Material>();
	icoMat->shader = icoShader;
	prisMat = std::make_shared<Cacao::Material>();
	prisMat->shader = prisShader;
	Cacao::ShaderUploadItem prismData_Tex;
	prismData_Tex.target = "texSample";
	prismData_Tex.data = prisTex.GetManagedAsset().get();
	prisMat->data.push_back(prismData_Tex);

	//Create camera manager
	cameraManager = std::make_shared<Cacao::Entity>("Camera Manager");
	cameraManager->SetActive(true);
	cameraManager->GetComponent<SussyScript>(cameraManager->MountComponent<SussyScript>())->SetActive(true);
	cameraManager->SetParent(world.rootEntity);
	apGuid = cameraManager->MountComponent<Cacao::AudioPlayer>();

	//Configure audio player
	std::shared_ptr<Cacao::AudioPlayer> audioPlayer = cameraManager->GetComponent<Cacao::AudioPlayer>(apGuid);
	audioPlayer->SetLooping(true);
	audioPlayer->SetGain(1.0f);
	audioPlayer->sound = bgm;

	//Generate icospheres
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist(0, ICOSPHERE_RANGE * 2);
	for(int i = 0; i < ICOSPHERE_COUNT; i++) {
		std::stringstream ss;
		ss << "Icosphere" << i;
		icospheres.push_back(std::make_shared<Cacao::Entity>(ss.str()));
		std::shared_ptr<Cacao::MeshComponent> mc = icospheres[i]->GetComponent<Cacao::MeshComponent>(icospheres[i]->MountComponent<Cacao::MeshComponent>());
		mc->SetActive(true);
		mc->mesh = icoMesh;
		mc->mat = icoMat;

		int x = dist(rng) - ICOSPHERE_RANGE, y = dist(rng) - ICOSPHERE_RANGE, z = dist(rng) - ICOSPHERE_RANGE;
		icospheres[i]->GetLocalTransform().SetPosition({x, y, z});
		icospheres[i]->SetActive(true);

		icospheres[i]->SetParent(world.rootEntity);
	}

	//Create prisms
	p1 = std::make_shared<Cacao::Entity>("Prism");
	p1->SetActive(true);
	std::shared_ptr<Cacao::MeshComponent> p1MC = p1->GetComponent<Cacao::MeshComponent>(p1->MountComponent<Cacao::MeshComponent>());
	p1->GetComponent<PingPong>(p1->MountComponent<PingPong>(glm::vec3 {2, 0, -2}, glm::vec3 {2, 0, 2}))->SetActive(true);
	p1->GetComponent<Spinner>(p1->MountComponent<Spinner>())->SetActive(true);
	p1MC->SetActive(true);
	p1MC->mesh = prisMesh;
	p1MC->mat = prisMat;
	p1->GetLocalTransform().SetScale({0.5f, 0.5f, 0.5f});
	p1->SetParent(world.rootEntity);
	p2 = std::make_shared<Cacao::Entity>("Prism");
	p2->SetActive(true);
	std::shared_ptr<Cacao::MeshComponent> p2MC = p2->GetComponent<Cacao::MeshComponent>(p2->MountComponent<Cacao::MeshComponent>());
	p2MC->SetActive(true);
	p2MC->mesh = prisMesh;
	p2MC->mat = prisMat;
	p2->GetLocalTransform().SetPosition({0, -3, 0});
	p2->GetLocalTransform().SetScale({1.0f, 1.0f, 1.0f});
	p2->GetLocalTransform().SetRotation({180, 0, 0});
	p2->SetParent(p1);

	//Set skybox
	world.skybox = sky;

	//Start audio playback
	audioPlayer->Play();
}

extern "C" {
EXPORT void _CacaoLaunch() {
	PlaygroundApp::GetInstance()->Launch();
}

EXPORT void _CacaoExiting() {
	PlaygroundApp::GetInstance()->Cleanup();
}
}