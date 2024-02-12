#pragma once

#include "Graphics/Shader.hpp"

#include "glad/gl.h"

#include <map>

namespace CacaoEngine {

    //OpenGL implementation of Shader (see Shader.hpp for method details)
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(std::string vertexSrc, std::string fragmentSrc);

        void Bind() override;
        void Unbind() override;
        void Compile() override;
        void Release() override;

        void UploadUniformMat4(std::string uniform, glm::mat4 value) override;
        void UploadUniformMat3(std::string uniform, glm::mat3 value) override;
        void UploadUniformBoolean(std::string uniform, bool value) override;
        void UploadUniformFloat(std::string uniform, float value) override;
        void UploadUniformFloat2(std::string uniform, glm::vec2 value) override;
        void UploadUniformFloat3(std::string uniform, glm::vec3 value) override;
        void UploadUniformFloat4(std::string uniform, glm::vec4 value) override;
        void UploadUniformInt(std::string uniform, int value) override;
        void UploadUniformInt2(std::string uniform, glm::ivec2 value) override;
        void UploadUniformInt3(std::string uniform, glm::ivec3 value) override;
        void UploadUniformInt4(std::string uniform, glm::ivec4 value) override;
    private:
        uint32_t compiledForm;
        std::string vertexShader;
        std::string fragmentShader;

        std::map<const char*, GLint> uniformLocations;
    };
}