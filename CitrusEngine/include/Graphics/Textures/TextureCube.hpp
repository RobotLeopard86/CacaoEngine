#pragma once

#include "BaseTexture.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <map>

namespace CitrusEngine {

	//Enum for defining all possible faces for cube
	//Yes I know that Model.hpp has basically this same enum but I don't care
	enum CubeFace {
		PosX = 0,
		NegX = 1,
		PosY = 2,
		NegY = 3,
		PosZ = 4,
		NegZ = 5
	};

	//Cubemap image data
	struct CubemapFace {
	public:
		unsigned char* dataBuffer;
		glm::ivec2 size;
		int numChannels;
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
        std::map<CubeFace, CubemapFace> faces;
    };
}