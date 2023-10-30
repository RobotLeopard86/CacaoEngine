#pragma once

#include "BaseTexture.hpp"

#include "glm/vec2.hpp"

#include <string>

namespace CitrusEngine {

	//2D texture
    //Must be implemented per-rendering API
    class Texture2D : public Texture {
    public:
        virtual ~Texture2D() {}

        //Use this texture
        virtual void Bind() {}
        //Don't use this texture
        virtual void Unbind() {}
        //Compile texture to be used later
        virtual void Compile() {}
        //Delete texture when no longer needed
        virtual void Release() {}
		
		//Creates texture for the current rendering API from a file
        static Texture2D* CreateFromFile(std::string filePath);
    protected:
        unsigned char* dataBuffer;
        glm::ivec2 imgSize;
        int numImgChannels;
    };
}