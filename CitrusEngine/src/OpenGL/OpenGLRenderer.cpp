#include "OpenGLRenderer.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include "glad/gl.h"

#include <string>

namespace CitrusEngine {

    OpenGLRenderer::OpenGLRenderer() {
        clearColor = glm::vec4(1.0);
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
        //Bind mesh and shader
        mesh->Bind();
        shader->Bind();

        int numVertices = mesh->GetVertices().size();
        int numIndices = mesh->GetIndices().size();

        float glVbContent[(sizeof(float) * 3) * numVertices];
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof(float) * 3) * numVertices, glVbContent);
        int glIbContent[(sizeof(int) * 3) * numIndices];
        glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (sizeof(int) * 3) * numIndices, glIbContent);

        //Draw geometry
        glDrawElements(GL_TRIANGLES, (numIndices * 3), GL_UNSIGNED_INT, nullptr);
        
        //Unbind mesh and shader
        mesh->Unbind();
        shader->Unbind();
    }

    Renderer* Renderer::CreateNativeRenderer(){
        return new OpenGLRenderer();
    }
}