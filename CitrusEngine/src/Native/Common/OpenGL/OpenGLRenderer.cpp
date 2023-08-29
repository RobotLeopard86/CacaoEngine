#include "Graphics/Renderer.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include "glad/gl.h"

#include "glm/gtc/matrix_transform.hpp"

#include <string>

//OpenGL implementation of Renderer (see Renderer.h for more details)

namespace CitrusEngine {

    Renderer::Renderer()
        : clearColor(glm::vec4(1.0f)), activeCam(nullptr) {}

    void Renderer::SetClearColor(glm::u8vec3 color) {
        clearColor = glm::vec4((float(color.r) / 256), (float(color.g) / 256), (float(color.b) / 256), 1.0);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }

    void Renderer::Clear(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::InitBackend(){
        Asserts::EngineAssert(!backendInitialized, "Glad is already intialized!");

        //Initialize Glad (OpenGL loader)
        int gladLoaderResponse = gladLoaderLoadGL();

        //Ensure Glad initialized correctly
        Asserts::EngineAssert(gladLoaderResponse, "Failed to initialize Glad!");

        backendInitialized = true;

        //Log GL info
        const char* glVendor = (const char*)glGetString(GL_VENDOR);
        const char* glVersion = (const char*)glGetString(GL_VERSION);
        const char* glRenderer = (const char*)glGetString(GL_RENDERER);
        std::string msg = "Citrus Engine OpenGL Info:\n  OpenGL v";
        msg = msg + glVersion + " provided by " + glVendor + ", running on " + glRenderer;
        Logging::EngineLog(LogLevel::Trace, msg);
    }

    void Renderer::ShutdownBackend(){
        Asserts::EngineAssert(backendInitialized, "Glad is not intialized!");

        //Unload Glad
        gladLoaderUnloadGL();

        backendInitialized = false;
    }

    void Renderer::RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader) {
        Asserts::EngineAssert(activeCam != nullptr, "Cannot render without an active camera!");

        //Bind mesh and shader
        mesh->Bind();
        shader->Bind();

        //Upload uniforms
        shader->UploadUniformMat4("transform", transform->GetTransformationMatrix());
        shader->UploadUniformMat4("camview", activeCam->GetViewProjectionMatrix());

        int numIndices = mesh->GetIndices().size() * 3;

        //Ensure further faces are drawn over by closer ones
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        //Draw geometry
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
        
        //Unbind mesh and shader
        mesh->Unbind();
        shader->Unbind();
    }

    void Renderer::SetCamera(Camera* cam){
        activeCam = cam;
    }
}