#pragma once

#include <string>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Shader {
    public:
        virtual ~Shader() {}

        //Use this shader
        virtual void Bind() {}
        //Don't use this shader
        virtual void Unbind() {}
        //Compile shader to be used later
        virtual void Compile() {}
        //Delete shader when no longer needed
        virtual void Release() {}

        //Is shader compiled?
        bool IsCompiled() { return compiled; }

        //Is shader bound?
        bool IsBound() { return bound; }

        //Uniform uploading functions

        //Uploads 4x4 float matrix
        virtual void UploadUniformMat4(std::string uniform, glm::mat4 value) {}
        //Uploads 3x3 float matrix
        virtual void UploadUniformMat3(std::string uniform, glm::mat3 value) {}
        //Uploads one boolean value
        virtual void UploadUniformBoolean(std::string uniform, bool value) {}
        //Uploads one float value
        virtual void UploadUniformFloat(std::string uniform, float value) {}
        //Uploads a two-component float vector
        virtual void UploadUniformFloat2(std::string uniform, glm::vec2 value) {}
        //Uploads a three-component float vector
        virtual void UploadUniformFloat3(std::string uniform, glm::vec3 value) {}
        //Uploads a four-component float vector
        virtual void UploadUniformFloat4(std::string uniform, glm::vec4 value) {}
        //Uploads one integer value
        virtual void UploadUniformInt(std::string uniform, int value) {}
        //Uploads a two-component integer vector
        virtual void UploadUniformInt2(std::string uniform, glm::ivec2 value) {}
        //Uploads a three-component integer vector
        virtual void UploadUniformInt3(std::string uniform, glm::ivec3 value) {}
        //Uploads a four-component integer vector
        virtual void UploadUniformInt4(std::string uniform, glm::ivec4 value) {}

        //Creates shader for the current rendering API
        static Shader* Create(std::string vertexShader, std::string fragmentShader);
    protected:
        bool compiled;
        bool bound;
    };
}