#pragma once

#include "Graphics/Textures/Texture2D.hpp"

#include "glad/gl.h"

namespace CacaoEngine {

    //OpenGL implementation of Texture2D (see Texture2D.hpp for method details)
    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(std::string filePath);
        ~OpenGLTexture2D();

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;
    private:
        GLuint compiledForm;
		GLenum format;
    };
}