#pragma once

#include "Texture.hpp"

#include "glm/vec2.hpp"

#include <string>

namespace Citrus {

	//2D texture
    //Must be implemented per-rendering API
    class Texture2D : public Texture {
    public:
		Texture2D(std::string filePath);

        //Use this texture
        void Bind() {}
        //Don't use this texture
        void Unbind() {}
        //Compile texture to be used later
        void Compile() {}
        //Delete texture when no longer needed
        void Release() {}
    protected:
        unsigned char* dataBuffer;
        glm::ivec2 imgSize;
        int numImgChannels;

		void* nativeData;
    };
}