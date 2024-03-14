#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "GLTexture2DData.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <filesystem>

//For my sanity
#define nd ((GLTexture2DData*)nativeData)

namespace Cacao {
	Texture2D::Texture2D(std::string filePath) {
		//Create native data
		nativeData = new GLTexture2DData();

		//Ensure stb_image flips image Y (because OpenGL)
		stbi_set_flip_vertically_on_load(true);

		//Load image
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

		compiled = false;
		bound = false;
		currentSlot = -1;

		//Determine image format
		if(numImgChannels == 1) {
            nd->format = GL_RED;
        } else if(numImgChannels == 3) {
            nd->format = GL_RGB;
        } else if(numImgChannels == 4) {
            nd->format = GL_RGBA;
        }
	}

	void Texture2D::Compile(){
		if(compiled){
            Logging::EngineLog("Cannot compile already compiled texture!", LogLevel::Error);
			return;
        }

		//Create texture object
		glGenTextures(1, &(nd->gpuID));
		
		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_2D, nd->gpuID);

		//Load image data into texture object
		glTexImage2D(GL_TEXTURE_2D, 0, nd->format, imgSize.x, imgSize.y, 0, nd->format, GL_UNSIGNED_BYTE, dataBuffer);

		//Apply texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Allow minimum detail mipmaps
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

		//Configure texture wrapping mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Generate mipmaps for texture
		glGenerateMipmap(GL_TEXTURE_2D);

		//Unbind texture object since we're done with it for now
		glBindTexture(GL_TEXTURE_2D, 0);
		
		compiled = true;
	}

	void Texture2D::Release() {
		if(!compiled){
            Logging::EngineLog("Cannot release uncompiled texture!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot release bound texture!", LogLevel::Error);
            return;
        }
        glDeleteTextures(1, &(nd->gpuID));
        compiled = false;
	}

	void Texture2D::Bind(int slot){
        if(!compiled){
            Logging::EngineLog("Cannot bind uncompiled texture!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot bind already bound texture!", LogLevel::Error);
            return;
        }
		//Bind the texture to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, nd->gpuID);
        bound = true;
    }

    void Texture2D::Unbind(){
        if(!compiled){
            Logging::EngineLog("Cannot unbind uncompiled texture!", LogLevel::Error);
            return;
        }
        if(!bound){
            Logging::EngineLog("Cannot unbind unbound texture!", LogLevel::Error);
            return;
        }
		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
        glBindTexture(GL_TEXTURE_2D, 0);
		currentSlot = -1;
        bound = false;
    }
}