#include "Graphics/Textures/Cubemap.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "GLCubemapData.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <filesystem>

//For my sanity
#define nd ((GLCubemapData*)nativeData)

namespace Cacao {
	Cubemap::Cubemap(std::vector<std::string> filePaths) {
		//Create native data
		nativeData = new GLCubemapData();

		for(std::string tex : filePaths){
			EngineAssert(std::filesystem::exists(tex), "Cannot create cubemap from nonexistent file!");
		}

		textures = filePaths;
		currentSlot = -1;
	}

	void Cubemap::Compile(){
		if(compiled){
            Logging::EngineLog("Cannot compile already compiled cubemap!", LogLevel::Error);
			return;
        }

		//Create texture object
		glGenTextures(1, &(nd->gpuID));
		
		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_CUBE_MAP, nd->gpuID);

		//Load image data into texture object
		for(unsigned int i = 0; i < textures.size(); i++){
			//Define fields for image loading
			int width, height, numChannels;

			//Make sure textures are flipped
			stbi_set_flip_vertically_on_load(true);

			//Load texture data from file
			unsigned char *data = stbi_load(textures[i].c_str(), &width, &height, &numChannels, 0);

			//Make sure we have data
			if(data) {
				//Add image data to cubemap
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				//Free data (we won't be needing it anymore)
				stbi_image_free(data);
			} else {
				//Free whatever junk we have
				stbi_image_free(data);
			}
		}

		//Apply cubemap filtering
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Configure wrapping mode
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//Unbind texture object since we're done with it for now
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		
		compiled = true;
	}

	void Cubemap::Release() {
		if(!compiled){
            Logging::EngineLog("Cannot release uncompiled cubemap!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot release bound cubemap!", LogLevel::Error);
            return;
        }
        glDeleteTextures(1, &(nd->gpuID));
        compiled = false;
	}

	void Cubemap::Bind(int slot){
        if(!compiled){
            Logging::EngineLog("Cannot bind uncompiled cubemap!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot bind already bound cubemap!", LogLevel::Error);
            return;
        }
		//Bind the texture to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, nd->gpuID);
        bound = true;
    }

    void Cubemap::Unbind(){
        if(!compiled){
            Logging::EngineLog("Cannot unbind uncompiled cubemap!", LogLevel::Error);
            return;
        }
        if(!bound){
            Logging::EngineLog("Cannot unbind unbound cubemap!", LogLevel::Error);
            return;
        }
		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		currentSlot = -1;
        bound = false;
    }
}