#pragma once

#include "BaseTexture.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace CitrusEngine {

	//Cubemap image data
	class CubemapFace {
	public:
		unsigned char* dataBuffer;
		glm::ivec2 size;
		int numChannels;

		CubemapFace(std::string path);
		~CubemapFace();
	};

	//Cubemap texture
    //Must be implemented per-rendering API
    class TextureCube : public Texture {
    public:
        virtual ~TextureCube() {}

        //Use this texture
        virtual void Bind() {}
        //Don't use this texture
        virtual void Unbind() {}
        //Compile texture to be used later
        virtual void Compile() {}
        //Delete texture when no longer needed
        virtual void Release() {}
		
		//Creates texture for the current rendering API from a file
        static TextureCube* CreateFromFile(std::string posX, std::string negX, std::string posY, std::string negY, std::string posZ, std::string negZ);
    protected:
		//Order: +X, -X, +Y, -Y, +Z, -Z
        CubemapFace *px, *nx, *py, *ny, *pz, *nz;
    };
}