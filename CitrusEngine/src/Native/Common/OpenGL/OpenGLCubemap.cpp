#include "Native/Common/OpenGL/OpenGLCubemap.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <filesystem>

namespace CacaoEngine {

	Cubemap* Cubemap::CreateFromFiles(std::vector<std::string> filePaths) {
		for(std::string tex : filePaths){
			Asserts::EngineAssert(std::filesystem::exists(tex), "Cannot create cubemap from nonexistent file!");
		}

		return new OpenGLCubemap(filePaths);
	}

	OpenGLCubemap::OpenGLCubemap(std::vector<std::string> filePaths) {
		textures = filePaths;
	}

	OpenGLCubemap::~OpenGLCubemap(){
		if(bound) Unbind();
		if(compiled) Release();
	}

	void OpenGLCubemap::Compile(){
		if(compiled){
			Logging::EngineLog(LogLevel::Error, "Cannot compile already compiled cubemap!");
			return;
		}

		//Create texture object
		glGenTextures(1, &compiledForm);
		
		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_CUBE_MAP, compiledForm);

		//Load image data into texture object
		for(unsigned int i = 0; i < textures.size(); i++){
			//Define fields for image loading
			int width, height, numChannels;

			//Make sure textures aren't flipped
			//Normally OpenGL expects them flipped but cubemaps don't need that
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

	void OpenGLCubemap::Release() {
		if(!compiled){
			Logging::EngineLog(LogLevel::Error, "Cannot release uncompiled cubemap!");
			return;
		}
		if(bound){
			Logging::EngineLog(LogLevel::Error, "Cannot release bound cubemap!");
			return;
		}
		glDeleteTextures(1, &compiledForm);
		compiled = false;
	}

	void OpenGLCubemap::Bind(){
		if(!compiled){
			Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled cubemap!");
			return;
		}
		if(bound){
			Logging::EngineLog(LogLevel::Error, "Cannot bind already bound cubemap!");
			return;
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, compiledForm);
		bound = true;
	}

	void OpenGLCubemap::Unbind(){
		if(!compiled){
			Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled cubemap!");
			return;
		}
		if(!bound){
			Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound cubemap!");
			return;
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		bound = false;
	}
}