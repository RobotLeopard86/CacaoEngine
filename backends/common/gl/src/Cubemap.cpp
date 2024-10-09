#include "Graphics/Textures/Cubemap.hpp"

#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "GLCubemapData.hpp"
#include "GLUtils.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <future>
#include <filesystem>

namespace Cacao {
	Cubemap::Cubemap(std::vector<std::string> filePaths)
	  : Texture(false) {
		//Create native data
		nativeData.reset(new CubemapData());

		for(std::string tex : filePaths) {
			CheckException(std::filesystem::exists(tex), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create cubemap from nonexistent file!");
		}

		textures = filePaths;
		currentSlot = -1;
	}

	std::shared_future<void> Cubemap::Compile() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL (ES) on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled cubemap!");

		//Create texture object
		glGenTextures(1, &(nativeData->gpuID));

		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_CUBE_MAP, nativeData->gpuID);

		//Load image data into texture object
		for(unsigned int i = 0; i < textures.size(); i++) {
			//Define fields for image loading
			int width, height, numChannels;

			//Make sure textures are flipped
			stbi_set_flip_vertically_on_load(true);

			//Load texture data from file
			unsigned char* data = stbi_load(textures[i].c_str(), &width, &height, &numChannels, 3);

			//Make sure we have data
			if(data) {
				//Add image data to cubemap
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				//Free data (we won't be needing it anymore)
				stbi_image_free(data);
			} else {
				//Free whatever junk we have
				stbi_image_free(data);

				CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to open cubemap face image file!")
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

		//Return an empty future
		return {};
	}

	void Cubemap::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL (ES) on the main thread
			//Try to invoke OpenGL (ES) and throw any exceptions back to the initial caller
			try {
				InvokeGL([this]() {
					this->Release();
				}).get();
				return;
			} catch(...) {
				std::rethrow_exception(std::current_exception());
			}
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled cubemap!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot release bound cubemap!");

		glDeleteTextures(1, &(nativeData->gpuID));
		compiled = false;
	}

	void Cubemap::Bind(int slot) {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot bind cubemap in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled cubemap!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound cubemap!");

		//Bind the texture to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, nativeData->gpuID);
		bound = true;
	}

	void Cubemap::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot unbind cubemap in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled cubemap!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound cubemap!");

		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		currentSlot = -1;
		bound = false;
	}
}