#pragma once

#include "Graphics/Renderer.h"

#include "glm/vec4.hpp"

namespace CitrusEngine {
    
    //OpenGL implementation of Renderer (see Renderer.h for method details)
    class OpenGLRenderer : public Renderer {
    public:
        OpenGLRenderer();

        void SetClearColor(glm::u8vec3 color) override;
        void Clear() override;
        void RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader) override;
        void ResizeViewport(int width, int height) override;
        void InitBackend() override;
        void SetCamera(Camera* cam) override;
    private:
        glm::vec4 clearColor;
        Camera* activeCam;
    };
}