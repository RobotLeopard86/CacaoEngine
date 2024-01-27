#pragma once

#include "Graphics/Textures/Cubemap.hpp"

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL implementation of Cubemap (see Cubemap.hpp for method details)
    class OpenGLCubemap : public Cubemap {
    public:
        OpenGLCubemap(std::vector<std::string> filePaths);
        ~OpenGLCubemap();

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;
    private:
        GLuint compiledForm;
    };
}