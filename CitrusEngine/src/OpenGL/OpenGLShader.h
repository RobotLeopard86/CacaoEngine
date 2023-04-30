#pragma once

#include "Graphics/Shader.h"

namespace CitrusEngine {

    //OpenGL implementation of Shader (see Shader.h for method details)
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(std::string vertexSrc, std::string fragmentSrc);

        void Bind() override;
        void Unbind() override;
        void Compile() override;
    private:
        uint32_t compiledForm;
        bool compiled;
        bool bound;
        std::string vertexShader;
        std::string fragmentShader;
    };
}