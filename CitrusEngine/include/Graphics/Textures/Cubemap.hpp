#pragma once

#pragma once

#include "BaseTexture.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace CitrusEngine {

	//Cubemap texture
    //Must be implemented per-rendering API
    class Cubemap : public Texture {
    public:
        virtual ~Cubemap() {}

        //Use this cubemap
        virtual void Bind() {}
        //Don't use this cubemap
        virtual void Unbind() {}
        //Compile cubemap to be used later
        virtual void Compile() {}
        //Delete cubemap when no longer needed
        virtual void Release() {}
		
		//Creates cubemap for the current rendering API from a file
		//Order of faces: +X, -X, +Y, -Y, +Z, -Z
        static Cubemap* CreateFromFiles(std::vector<std::string> filePaths);
    protected:
        std::vector<std::string> textures;
    };
}