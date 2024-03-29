#pragma once

#include "Texture.hpp"
#include "Utilities/MiscUtils.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace Cacao {

	//Cubemap texture
    //Must be implemented per-rendering API
    class Cubemap : public Texture {
    public:
		//Order of faces: +X, -X, +Y, -Y, +Z, -Z
        Cubemap(std::vector<std::string> filePaths);

		~Cubemap(){
			if(bound) Unbind();
			if(compiled) Release();
			delete nativeData;
		}
		
        //Attach this cubemap to the specified slot
        void Bind(int slot) override;
        //Detach this cubemap
        void Unbind() override;
        //Compile cubemap to be used later
        std::future<void> Compile() override;
        //Delete cubemap when no longer needed
        void Release() override;
    protected:
        std::vector<std::string> textures;

		NativeData* nativeData;
    };
}