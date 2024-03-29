#include "Cacao.hpp"

#include <chrono>

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

	void Launch();

	void Cleanup() {
		delete mat;
		delete shader;
		delete mesh;
		delete this;
	}

	UUIDv4::UUID GetBobUUID() {
		return bob.uuid;
	}
private:
	static PlaygroundApp* instance;
	static bool instanceExists;

	Cacao::Shader* shader;
	Cacao::Material* mat;
	Cacao::Mesh* mesh;

	Cacao::Entity bob;
};

std::string Vec3ToString(const glm::vec3& vec, bool format = true){
	std::stringstream ss;
	if(format) ss << "{ ";
	ss << vec.x << ", " << vec.y << ", " << vec.z;
	if(format) ss << " }";
	return ss.str();
}

class SussyScript : public Cacao::Script {
public:
	void OnActivate() override {
		Cacao::Logging::ClientLog("I'm awake!");
	}
	void OnDeactivate() override {
		Cacao::Logging::ClientLog("I'm asleep!");
	}
	void OnTick(double timestep) override {
		Cacao::World& world = Cacao::WorldManager::GetInstance()->GetActiveWorld();
		Cacao::PerspectiveCamera* cam = static_cast<Cacao::PerspectiveCamera*>(world.cam);
		count++;
		glm::vec3 camRotChange = glm::vec3(0.0f);
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_J)){
            camRotChange.y -= 0.5f;
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_K)){
            camRotChange.y += 0.5f;
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_Y)){
            camRotChange.x += 0.5f;
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_U)){
            camRotChange.x -= 0.5f;
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_X)){
            camRotChange.z += 0.5f;
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_C)){
            camRotChange.z -= 0.5f;
        }

        currentRot = cam->GetRotation();
        glm::vec3 pastRot = glm::vec3(currentRot);
        currentRot += camRotChange;
        
        if(currentRot.x < -89.99){
            currentRot.x = -89.99f;
        }
        if(currentRot.x > 89.99) {
            currentRot.x = 89.99f;
        }
        if(currentRot.y < 0){
            currentRot.y = 360.0f;
        }
        if(currentRot.y > 360) {
            currentRot.y = 0.0f;
        }
        if(currentRot.z < 0){
            currentRot.z = 360.0f;
        }
        if(currentRot.z > 360) {
            currentRot.z = 0.0f;
        }

        glm::vec3 posChange = glm::vec3(0.0f);
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_W)){
            posChange += cam->GetFrontVector() * 5.0f * float(timestep);
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_S)){
            posChange -= cam->GetFrontVector() * 5.0f * float(timestep);
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_D)){
            posChange += cam->GetRightVector() * 5.0f * float(timestep);
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_A)){
            posChange -= cam->GetRightVector() * 5.0f * float(timestep);
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_E)){
            posChange += cam->GetUpVector() * 5.0f * float(timestep);
        }
        if(Cacao::Input::GetInstance()->IsKeyPressed(CACAO_KEY_Q)){
            posChange -= cam->GetUpVector() * 5.0f * float(timestep);
        }

        currentPos = cam->GetPosition() + posChange;

        cam->SetRotation(Orientation(currentRot));
        cam->SetPosition(currentPos);

		std::stringstream msg;
		msg << "Camera position updated to " << Vec3ToString(currentPos);
		Cacao::Logging::ClientLog(msg.str());
	}
private:
	int count;
	glm::vec3 currentRot, currentPos;
};

PlaygroundApp* PlaygroundApp::instance = nullptr;
bool PlaygroundApp::instanceExists = false;

std::string Vec4ToString(const glm::vec4& vec, bool format = true){
	std::stringstream ss;
	if(format) ss << "{ ";
	ss << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
	if(format) ss << " }";
	return ss.str();
}

std::string MatRow(std::string vecVal, int longest){
	std::stringstream ss;
	ss << "| " << vecVal;
	for(int i = 0; i < (longest - 2 - vecVal.size()); i++) ss << " ";
	ss << " |\n";
	return ss.str();
}

std::string Mat4ToString(const glm::mat4& mat){
	std::stringstream ss;
	std::string r1 = Vec4ToString(mat[0], false), r2 = Vec4ToString(mat[1], false), r3 = Vec4ToString(mat[2], false), r4 = Vec4ToString(mat[3], false);
	int longest = r1.size();
	if(r2.size() > longest) longest = r2.size();
	if(r3.size() > longest) longest = r2.size();
	if(r4.size() > longest) longest = r2.size();
	longest += 2;
	for(int i = 0; i < longest; i++) ss << "=";
	ss << "\n";
	ss << MatRow(r1, longest);
	ss << MatRow(r2, longest);
	ss << MatRow(r3, longest);
	ss << MatRow(r4, longest);
	for(int i = 0; i < longest; i++) ss << "=";
	return ss.str();
}

void PlaygroundApp::Launch() {
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

	Cacao::Model cube("assets/models/icosphere.obj");
	mesh = cube.ExtractMesh("Icosphere");
	std::future<void> meshFuture = Cacao::Engine::GetInstance()->GetThreadPool().submit_task([this]() {
		this->mesh->Compile();
	});

	mat = new Cacao::Material();
	mat->shader = shader;

	std::shared_ptr<Cacao::MeshComponent> mc = std::make_shared<Cacao::MeshComponent>();
	mc->SetActive(true);
	mc->mesh = mesh;
	mc->mat = mat;

	meshFuture.wait();
	shaderFuture.wait();

	bob.active = true;
	bob.components.push_back(ss);
	bob.components.push_back(mc);

	world.worldTree.children.push_back(Cacao::TreeItem<Cacao::Entity>(bob));
}

extern "C" {
	void _CacaoLaunch() {
		PlaygroundApp::GetInstance()->Launch();
	}

	void _CacaoExiting() {
		PlaygroundApp::GetInstance()->Cleanup();
	}
}