#pragma once

#include "Graphics/Skybox.h"

namespace CitrusEngine {
    //OpenGL implementation of Skybox (see Skybox.h for method details)
    class OpenGLSkybox : public Skybox {
    public:
        OpenGLSkybox(std::string texturePath);
        ~OpenGLSkybox();

        void Draw() override;
    };
}