#include "Native/Common/OpenGL/OpenGLTextureCube.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <filesystem>

namespace CitrusEngine {

	TextureCube* TextureCube::CreateFromFile(std::string posX, std::string negX, std::string posY, std::string negY, std::string posZ, std::string negZ) {
		return new OpenGLTextureCube(posX, negX, posY, negY, posZ, negZ);
	}

	CubemapFace::CubemapFace(std::string path){
		stbi_set_flip_vertically_on_load(true);
		dataBuffer = stbi_load(path.c_str(), &size.x, &size.y, &numChannels, 0);
	}

	CubemapFace::~CubemapFace(){
		stbi_image_free(dataBuffer);
	}

	OpenGLTextureCube::OpenGLTextureCube(std::string posX, std::string negX, std::string posY, std::string negY, std::string posZ, std::string negZ) {
		compiled = false;
		bound = false;

		//Confirm existence of face paths
		Asserts::EngineAssert(std::filesystem::exists(posX), "Cannot create cube texture because +X face can not be created from nonexistent file!");
		Asserts::EngineAssert(std::filesystem::exists(negX), "Cannot create cube texture because -X face can not be created from nonexistent file!");
		Asserts::EngineAssert(std::filesystem::exists(posY), "Cannot create cube texture because +Y face can not be created from nonexistent file!");
		Asserts::EngineAssert(std::filesystem::exists(negY), "Cannot create cube texture because -Y face can not be created from nonexistent file!");
		Asserts::EngineAssert(std::filesystem::exists(posZ), "Cannot create cube texture because +Z face can not be created from nonexistent file!");
		Asserts::EngineAssert(std::filesystem::exists(negZ), "Cannot create cube texture because -Z face can not be created from nonexistent file!");

		//Store cubemap faces (created as they are added to vector)
		px = new CubemapFace(posX);
		nx = new CubemapFace(negX);
		py = new CubemapFace(posY);
		ny = new CubemapFace(negY);
		pz = new CubemapFace(posZ);
		nz = new CubemapFace(negZ);
	}

	OpenGLTextureCube::~OpenGLTextureCube(){
		if(bound) Unbind();
		if(compiled) Release();

		//Delete cubemap face pointers
		delete px;
		delete nx;
		delete py;
		delete ny;
		delete pz;
		delete nz;
	}

	void OpenGLTextureCube::Compile(){
		if(compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot compile already compiled texture!");
			return;
        }

		//Create texture object
		glGenTextures(1, &compiledForm);
		
		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_CUBE_MAP, compiledForm);

		//Load image data into texture object from faces

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, (px->numChannels == 3 ? GL_RGB : (px->numChannels == 4 ? GL_RGBA : GL_RED)), px->size.x, px->size.y, 0, (px->numChannels == 3 ? GL_RGB : (px->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, px->dataBuffer);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, (nx->numChannels == 3 ? GL_RGB : (nx->numChannels == 4 ? GL_RGBA : GL_RED)), nx->size.x, nx->size.y, 0, (nx->numChannels == 3 ? GL_RGB : (nx->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, nx->dataBuffer);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, (py->numChannels == 3 ? GL_RGB : (py->numChannels == 4 ? GL_RGBA : GL_RED)), py->size.x, py->size.y, 0, (py->numChannels == 3 ? GL_RGB : (py->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, py->dataBuffer);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, (ny->numChannels == 3 ? GL_RGB : (ny->numChannels == 4 ? GL_RGBA : GL_RED)), ny->size.x, ny->size.y, 0, (ny->numChannels == 3 ? GL_RGB : (ny->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, ny->dataBuffer);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, (pz->numChannels == 3 ? GL_RGB : (pz->numChannels == 4 ? GL_RGBA : GL_RED)), pz->size.x, pz->size.y, 0, (pz->numChannels == 3 ? GL_RGB : (pz->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, pz->dataBuffer);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, (nz->numChannels == 3 ? GL_RGB : (nz->numChannels == 4 ? GL_RGBA : GL_RED)), nz->size.x, nz->size.y, 0, (nz->numChannels == 3 ? GL_RGB : (nz->numChannels == 4 ? GL_RGBA : GL_RED)), GL_UNSIGNED_BYTE, nz->dataBuffer);

		//Apply texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//Unbind texture object since we're done with it for now
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		compiled = true;
	}

	void OpenGLTextureCube::Release() {
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

	void OpenGLTextureCube::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled texture!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound texture!");
            return;
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, compiledForm);
        bound = true;
    }

    void OpenGLTextureCube::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled texture!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound texture!");
            return;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        bound = false;
    }
}
