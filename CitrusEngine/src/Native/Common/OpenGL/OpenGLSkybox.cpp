#include "Native/Common/OpenGL/OpenGLSkybox.hpp"

#include <filesystem>

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

#include "Graphics/Texture.hpp"

#include "Models/Model.hpp"

#include "Utilities/StateManager.hpp"

#include "stb_image.h"

#include "glad/gl.h"

namespace CitrusEngine {

    //Initialize static skybox members to nullptr by default
    Shader* Skybox::skyboxShader = nullptr;
    Model* Skybox::skybox = nullptr;
    bool Skybox::staticMembersInitialized = false;

    Skybox* Skybox::Create(std::string texturePath){
        Asserts::EngineAssert(staticMembersInitialized, "You must initialize common skybox resources before creating a skybox!");

        return new OpenGLSkybox(texturePath);
    }

    void Skybox::InitializeResources(std::string modelPath){
        if(staticMembersInitialized) {
            Logging::EngineLog(LogLevel::Warn, "Ignoring redundant call to Skybox::InitializeResources, method already called.");
            return;
        }

        Asserts::EngineAssert(std::filesystem::exists(modelPath), "Cannot initialize skybox resources with nonexistent model path!");

        //Define shader code
        std::string vertCode = R"(
            #version 330 core

            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec2 tc;

            out vec2 texCoords;

            uniform mat4 transform;
            uniform mat4 camview;

            void main() {
                texCoords = tc;    
                gl_Position = transform * camview * vec4(pos, 1.0);
            }
        )";
        std::string fragCode = R"(
            #version 330 core
            out vec4 color;

            in vec2 texCoords;

            uniform sampler2D tex;

            void main()
            {    
                color = texture(tex, texCoords);
            }
        )";

        //Create skybox shader
        skyboxShader = Shader::Create(vertCode, fragCode);
        skyboxShader->Compile();

        //Load skybox model
        skybox = new Model(modelPath);

        staticMembersInitialized = true;
    }

    OpenGLSkybox::OpenGLSkybox(std::string texturePath){
        bool texExists = std::filesystem::exists(texturePath);
        Asserts::EngineAssert(texExists, "Cannot create skybox from nonexistent texture file!");

        //Create texture and transform assets
        tex = Texture::CreateFromFile(texturePath);
        tex->Compile();
        transform = new Transform({0, 0, 0}, {0, 0, 0}, {1000, 1000, 1000});
    }

    OpenGLSkybox::~OpenGLSkybox(){
        //Release texture assets
        tex->Release();

        delete tex;
        delete transform;
    }

    void OpenGLSkybox::Draw(){
        skyboxShader->Bind();

        glm::mat4 camVPM = StateManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
        camVPM = glm::mat4(glm::mat3(camVPM));

        skyboxShader->UploadUniformMat4("transform", transform->GetTransformationMatrix());
        skyboxShader->UploadUniformMat4("camview", camVPM);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        skybox->DrawMeshPure("SKYBOX");
        glDepthMask(GL_TRUE);

        skyboxShader->Unbind();
    }
}