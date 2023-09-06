#pragma once

#include "Graphics/Texture.h"

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL implementation of Texture (see Texture.h for method details)
    class OpenGLTexture : public Texture {
    public:
        OpenGLTexture(unsigned char* dataBuf, bool dataFromSTB, glm::i32vec2 size);
        ~OpenGLTexture();

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;
    private:
        GLuint compiledForm;
        bool compiled;
        bool bound;
        bool bufFromSTB;
        unsigned char* dataBuffer;
        glm::i32vec2 imgSize;
    };
}