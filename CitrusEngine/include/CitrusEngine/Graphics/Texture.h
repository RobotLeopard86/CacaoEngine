#pragma once

#include "glm/vec2.hpp"

#include <string>

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Texture {
    public:
        //Use this texture
        virtual void Bind() {}

        //Don't use this texture
        virtual void Unbind() {}

        //Compile texture to be used later
        virtual void Compile() {}

        //Delete texture when no longer needed
        virtual void Release() {}

        //Creates texture for the current rendering API (from a file path)
        static Texture* CreateTextureFromFile(std::string filePath);
        //Creates texture for the current rendering API (from a pre-existing data stream)
        static Texture* CreateTextureFromData(unsigned char* dataBuffer, glm::i32vec2 size);
    };
}