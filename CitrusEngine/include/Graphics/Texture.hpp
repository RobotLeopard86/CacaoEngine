#pragma once

#include "glm/vec2.hpp"

#include <string>

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Texture {
    public:
        virtual ~Texture() {}

        //Use this texture
        virtual void Bind() {}
        //Don't use this texture
        virtual void Unbind() {}
        //Compile texture to be used later
        virtual void Compile() {}
        //Delete texture when no longer needed
        virtual void Release() {}

        //Is texture compiled?
        bool IsCompiled() { return compiled; }

        //Is texture bound?
        bool IsBound() { return bound; }

        //Creates texture for the current rendering API from a file
        static Texture* CreateFromFile(std::string filePath);
        //Creates texture for the current rendering API from pre-existing data
        static Texture* CreateFromData(unsigned char* data, glm::ivec2 size, int numChannels);
    protected:
        bool compiled;
        bool bound;
    };
}