#pragma once

#include "Graphics/Skybox.hpp"

namespace CitrusEngine {
    //OpenGL implementation of Skybox (see Skybox.hpp for method details)
    class OpenGLSkybox : public Skybox {
    public:
        OpenGLSkybox(std::string texturePath);
        ~OpenGLSkybox();

        void Draw() override;
    };
}