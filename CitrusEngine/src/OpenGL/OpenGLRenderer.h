#pragma once

#include "Graphics/Renderer.h"

#include "glm/vec4.hpp"

namespace CitrusEngine {
    
    //OpenGL implementation of Renderer (see Renderer.h for method details)
    class OpenGLRenderer : public Renderer {
    public:
        OpenGLRenderer();

        void SetClearColor_Impl(glm::u8vec3 color) override;
        void Clear_Impl() override;
        void RenderGeometry_Impl(Mesh* mesh, Transform* transform, Shader* shader) override;
        void ResizeViewport_Impl(int width, int height) override;
        void InitBackend_Impl() override;
        void SetCamera_Impl(Camera* cam) override;
    private:
        glm::vec4 clearColor;
        Camera* activeCam;
    };
}