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

		//Create cubemap face objects
		CubemapFace posXFace, negXFace, posYFace, negYFace, posZFace, negZFace;
		posXFace.dataBuffer = stbi_load(posX.c_str(), &posXFace.size.x, &posXFace.size.y, &posXFace.numChannels, 0);
		negXFace.dataBuffer = stbi_load(negX.c_str(), &negXFace.size.x, &negXFace.size.y, &negXFace.numChannels, 0);
		posYFace.dataBuffer = stbi_load(posY.c_str(), &posYFace.size.x, &posYFace.size.y, &posYFace.numChannels, 0);
		negYFace.dataBuffer = stbi_load(negY.c_str(), &negYFace.size.x, &negYFace.size.y, &negYFace.numChannels, 0);
		posZFace.dataBuffer = stbi_load(posZ.c_str(), &posZFace.size.x, &posZFace.size.y, &posZFace.numChannels, 0);
		negZFace.dataBuffer = stbi_load(negZ.c_str(), &negZFace.size.x, &negZFace.size.y, &negZFace.numChannels, 0);

		//Store cubemap faces
		faces.insert_or_assign(CubeFace::PosX, posXFace);
		faces.insert_or_assign(CubeFace::NegX, negXFace);
		faces.insert_or_assign(CubeFace::PosY, posYFace);
		faces.insert_or_assign(CubeFace::NegY, negYFace);
		faces.insert_or_assign(CubeFace::PosZ, posZFace);
		faces.insert_or_assign(CubeFace::NegZ, negZFace);

		//Determine face formats
		for(int i = 0; i < 5; i++){
			int numChannels = faces.at(static_cast<CubeFace>(i)).numChannels;

			GLenum format;
			if(numChannels == 1) {
				format = GL_RED;
			} else if(numChannels == 3) {
				format = GL_RGB;
			} else if(numChannels == 4) {
				format = GL_RGBA;
			}

			faceFormats.insert_or_assign(static_cast<CubeFace>(i), format);
		}
	}

	OpenGLTextureCube::~OpenGLTextureCube(){
		if(bound) Unbind();
		if(compiled) Release();

		for(auto it = faces.begin(); it != faces.end(); it++){
			delete it->second.dataBuffer;
		}
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
		for(int i = 0; i < 5; i++){
			//Load image data into texture object
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, faceFormats.at(static_cast<CubeFace>(i)), faces.at(static_cast<CubeFace>(i)).size.x, faces.at(static_cast<CubeFace>(i)).size.y, 0, faceFormats.at(static_cast<CubeFace>(i)), GL_UNSIGNED_BYTE, faces.at(static_cast<CubeFace>(i)).dataBuffer);

			//Apply texture filtering
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//Configure texture wrapping mode
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
		}

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
