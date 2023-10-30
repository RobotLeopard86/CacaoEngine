#include "Native/Common/OpenGL/OpenGLTexture2D.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <filesystem>

namespace CitrusEngine {

	Texture2D* Texture2D::CreateFromFile(std::string filePath) {
		Asserts::EngineAssert(std::filesystem::exists(filePath), "Cannot create 2D texture from nonexistent file!");

		return new OpenGLTexture2D(filePath);
	}

	OpenGLTexture2D::OpenGLTexture2D(std::string filePath) {
		//Ensure stb_image flips image Y (because OpenGL)
		stbi_set_flip_vertically_on_load(true);

		//Load image
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

		compiled = false;
		bound = false;

		//Determine image format
		if(numImgChannels == 1) {
            format = GL_RED;
        } else if(numImgChannels == 3) {
            format = GL_RGB;
        } else if(numImgChannels == 4) {
            format = GL_RGBA;
        }
	}

	OpenGLTexture2D::~OpenGLTexture2D(){
		if(bound) Unbind();
		if(compiled) Release();

		delete dataBuffer;
	}

	void OpenGLTexture2D::Compile(){
		if(compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot compile already compiled texture!");
			return;
        }

		//Create texture object
		glGenTextures(1, &compiledForm);
		
		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_2D, compiledForm);

		//Load image data into texture object
		glTexImage2D(GL_TEXTURE_2D, 0, format, imgSize.x, imgSize.y, 0, format, GL_UNSIGNED_BYTE, dataBuffer);

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

	void OpenGLTexture2D::Release() {
		if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot release uncompiled texture!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot release bound texture!");
            return;
        }
        glDeleteTextures(1, &compiledForm);
        compiled = false;
	}

	void OpenGLTexture2D::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled texture!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound texture!");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, compiledForm);
        bound = true;
    }

    void OpenGLTexture2D::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled texture!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound texture!");
            return;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        bound = false;
    }
}