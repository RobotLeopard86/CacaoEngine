#pragma once

#include "Graphics/Texture.h"

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL implementation of Texture (see Texture.h for method details)
    class OpenGLTexture : public Texture {
    public:
        OpenGLTexture(unsigned char* dataBuf, glm::ivec2 size, int numChannels);
        ~OpenGLTexture();

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;
    private:
        GLuint compiledForm;
        bool compiled;
        bool bound;
        unsigned char* dataBuffer;
        glm::ivec2 imgSize;
        int numImgChannels;
    };
}