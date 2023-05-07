#include "OpenGLRenderer.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include "glad/gl.h"

#include "glm/gtc/matrix_transform.hpp"

#include <string>

namespace CitrusEngine {

    OpenGLRenderer::OpenGLRenderer() {
        clearColor = glm::vec4(1.0);
        activeCam = nullptr;
    }

    void OpenGLRenderer::SetClearColor_Impl(glm::u8vec3 color) {
        clearColor = glm::vec4((float(color.r) / 256), (float(color.g) / 256), (float(color.b) / 256), 1.0);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }

    void OpenGLRenderer::Clear_Impl(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::ResizeViewport_Impl(int width, int height){
        glViewport(0, 0, width, height);
    }

    void OpenGLRenderer::InitBackend_Impl(){
        //Initialize Glad (OpenGL loader)
        bool gladSuccessfulInit = gladLoaderLoadGL();

        //Ensure Glad initialized correctly
        Asserts::EngineAssert(gladSuccessfulInit, "Failed to initialize Glad!");

        //Log GL info
        const char* glVendor = (const char*)glGetString(GL_VENDOR);
        const char* glVersion = (const char*)glGetString(GL_VERSION);
        const char* glRenderer = (const char*)glGetString(GL_RENDERER);
        std::string msg = "Citrus Engine OpenGL Info:\n  OpenGL v";
        msg = msg + glVersion + " provided by " + glVendor + ", running on " + glRenderer;
        Logging::EngineLog(LogLevel::Trace, msg);
    }

    void OpenGLRenderer::RenderGeometry_Impl(Mesh* mesh, Transform* transform, Shader* shader) {
        Asserts::EngineAssert(activeCam != nullptr, "Cannot render without an active camera!");

        //Bind mesh and shader
        mesh->Bind();
        shader->Bind();

        shader->UploadUniformMat4("transform", glm::translate(glm::mat4(1.0), transform->pos));
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

    void OpenGLRenderer::SetCamera_Impl(Camera* cam){
        activeCam = cam;
    }

    Renderer* Renderer::CreateNativeRenderer(){
        return new OpenGLRenderer();
    }
}