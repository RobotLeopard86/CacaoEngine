#include "CitrusEngine.hpp"

#include <string>
#include <sstream>

using namespace CitrusEngine;

class PlaygroundClient : public CitrusClient {
public:
    PlaygroundClient() { windowTitle = "Citrus Playground"; windowSize = {1280, 720}; }

    std::string Vec3ToString(glm::vec3 vec){
        return "{ X: " + std::to_string(vec.x) + ", Y: " + std::to_string(vec.y) + ", Z: " + std::to_string(vec.z) + " }";
    }

    void ClientOnStartup() override {
        mdl = new Model("CitrusPlayground/assets/model.fbx");

        transform = new Transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1});

        std::string vertexShaderSource = R"(
            #version 330 core
            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec2 tc;

            out vec2 texCoords;

            uniform mat4 model;
            uniform mat4 view;
			uniform mat4 projection;

            void main() {
                texCoords = tc;    
                gl_Position = projection * view * model * vec4(pos, 1.0f);
            }
        )";
        
        std::string fragmentShaderSource = R"(
            #version 330 core
            out vec4 color;

            in vec2 texCoords;

            uniform sampler2D tex;

            void main()
            {    
                color = texture(tex, texCoords);
            }
        )";
        
        shader = Shader::Create(vertexShaderSource, fragmentShaderSource);
        shader->Compile();

        tex = Texture2D::CreateFromFile("CitrusPlayground/assets/model.fbm/color.png");
        tex->Compile();

		skyTex = Cubemap::CreateFromFiles({"CitrusPlayground/assets/sky/right.jpg", "CitrusPlayground/assets/sky/left.jpg", "CitrusPlayground/assets/sky/top.jpg", "CitrusPlayground/assets/sky/bottom.jpg", "CitrusPlayground/assets/sky/front.jpg", "CitrusPlayground/assets/sky/back.jpg"});
		skyTex->Compile();

		sky = Skybox::Create(skyTex);

        cam = new PerspectiveCamera(60, GetWindow()->GetSize());
        cam->SetPosition({ 0, 0, 3 });
        cam->SetClearColor({ 255, 255, 255 });

        StateManager::GetInstance()->SetActiveCamera(cam);

        uiDrawConsumer = new EventConsumer(BIND_MEMBER_FUNC(PlaygroundClient::OnImGuiDraw));
        EventManager::GetInstance()->SubscribeConsumer("ImGuiDraw", uiDrawConsumer);
    }

    void ClientOnShutdown() override {
        //Release texture and shader resources
        shader->Release();
        tex->Release();
		skyTex->Release();

        delete mdl;
        delete tex;
        delete transform;
        delete shader;
		delete sky;
		delete skyTex;
        delete uiDrawConsumer;
    }

    void ClientOnDynamicTick(double timestep) override {
        glm::vec3 camRotChange = glm::vec3(0.0f);
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_J)){
            camRotChange.y -= 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_K)){
            camRotChange.y += 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_Y)){
            camRotChange.x += 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_U)){
            camRotChange.x -= 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_X)){
            camRotChange.z += 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_C)){
            camRotChange.z -= 0.5f;
        }
        currentRot = cam->GetRotation().ToVec3();
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
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_W)){
            posChange += cam->GetFrontVector() * 1000.0f * float(timestep);
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_S)){
            posChange -= cam->GetFrontVector() * 1000.0f * float(timestep);
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_D)){
            posChange += cam->GetRightVector() * 1000.0f * float(timestep);
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_A)){
            posChange -= cam->GetRightVector() * 1000.0f * float(timestep);
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_E)){
            posChange += cam->GetUpVector() * 1000.0f * float(timestep);
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_Q)){
            posChange -= cam->GetUpVector() * 1000.0f * float(timestep);
        }

        currentPos = cam->GetPosition() + posChange;

        cam->SetRotation(Orientation(currentRot));
        cam->SetPosition(currentPos);

        tex->Bind();
        mdl->DrawMesh("Shape", shader, transform);
        tex->Unbind();

		sky->Draw();
    }
    void ClientOnFixedTick() override {}

    void OnImGuiDraw(Event& e){
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);

        std::stringstream sscam;
        sscam << "Camera is at " << Vec3ToString(currentPos);
		std::stringstream sstilt;
        sstilt << "Camera tilt: " << currentRot.x;
        std::stringstream sspan;
        sspan << "Camera pan: " << currentRot.y;
		std::stringstream ssclt;
        ssclt << "Camera look target: " << Vec3ToString(cam->GetLookTarget());
		std::stringstream ssf;
        ssf << "Camera front vec: " << Vec3ToString(cam->GetFrontVector());
		std::stringstream ssu;
        ssu << "Camera up vec: " << Vec3ToString(cam->GetUpVector());
		std::stringstream ssr;
        ssr << "Camera right vec: " << Vec3ToString(cam->GetRightVector());
		std::stringstream ssvs;
        ssvs << "VSync is currently " << (GetWindow()->IsVSyncEnabled() ? "on" : "off");

        ImGui::Begin("Camera Debug Info");
        ImGui::Text("%s", sscam.str().c_str());
        ImGui::Text("%s", sstilt.str().c_str());
		ImGui::Text("%s", sspan.str().c_str());
		ImGui::Text("%s", ssclt.str().c_str());
		ImGui::Text("%s", ssf.str().c_str());
		ImGui::Text("%s", ssu.str().c_str());
		ImGui::Text("%s", ssr.str().c_str());
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(110, 300), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(175, 75), ImGuiCond_Once);

		ImGui::Begin("VSync Management");
        if(ImGui::Button("Toggle VSync")){
            GetWindow()->SetVSyncEnabled(!GetWindow()->IsVSyncEnabled());
        }
        ImGui::Text("%s", ssvs.str().c_str());
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(110, 400), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(300, 130), ImGuiCond_Once);

        float posBuf[3] = { transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z };
        float rotBuf[3] = { transform->GetRotation().x, transform->GetRotation().y, transform->GetRotation().z };
        float sclBuf[3] = { transform->GetScale().x, transform->GetScale().y, transform->GetScale().z };

        ImGui::Begin("Transform Controls"); 
        ImGui::InputFloat3("Position", posBuf);
        ImGui::InputFloat3("Rotation", rotBuf);
        ImGui::InputFloat3("Scale", sclBuf);
        ImGui::Spacing();

        if(ImGui::Button("Uniform Scaling")) ImGui::OpenPopup("Uniform Scaling");

        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(375, 100));
        if(ImGui::BeginPopupModal("Uniform Scaling", &open)){
            static float unifValue = 0.0f;
            ImGui::InputFloat("New Scale Value", &unifValue);

            if (ImGui::Button("Close")) {
                unifValue = 0.0f;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Save & Close")) {
                sclBuf[0] = unifValue;
                sclBuf[1] = unifValue;
                sclBuf[2] = unifValue;
                unifValue = 0.0f;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        transform->SetPosition({ posBuf[0], posBuf[1], posBuf[2] });
        transform->SetRotation({ rotBuf[0], rotBuf[1], rotBuf[2] });
        transform->SetScale({ sclBuf[0], sclBuf[1], sclBuf[2] });
    }
private:
    Model* mdl;
    Transform* transform;
    Shader* shader;
    Texture2D* tex;
	Cubemap* skyTex;
	Skybox* sky;
    PerspectiveCamera* cam;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u32vec3> indices;

    glm::vec3 currentPos, currentRot;

    EventConsumer* uiDrawConsumer;
};

CitrusClient* CreateClient() {
    return new PlaygroundClient();
}