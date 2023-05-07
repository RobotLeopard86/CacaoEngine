#pragma once

#include <string>

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

        //Creates shader for the current rendering API
        static Shader* CreateShader(std::string vertexShader, std::string fragmentShader);
    };
}