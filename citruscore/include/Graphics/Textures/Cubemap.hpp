#pragma once

#pragma once

#include "BaseTexture.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace Citrus {

	//Cubemap texture
    //Must be implemented per-rendering API
    class Cubemap : public Texture {
    public:
		//Order of faces: +X, -X, +Y, -Y, +Z, -Z
        Cubemap(std::vector<std::string> filePaths);
		
        //Use this cubemap
        void Bind() {}
        //Don't use this cubemap
        void Unbind() {}
        //Compile cubemap to be used later
        void Compile() {}
        //Delete cubemap when no longer needed
        void Release() {}
    protected:
        std::vector<std::string> textures;

		void* nativeData;
    };
}