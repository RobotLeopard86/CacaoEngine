#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "ESTexture2DData.hpp"
#include "ESUtils.hpp"

#include "stb_image.h"

#include "glad/gles2.h"

#include <future>
#include <filesystem>

//For my sanity
#define nd ((ESTexture2DData*)nativeData)

namespace Cacao {
	Texture2D::Texture2D(std::string filePath)
	  : Texture(false) {
		//Create native data
		nativeData = new ESTexture2DData();

		//Ensure stb_image flips image Y (because OpenGL)
		stbi_set_flip_vertically_on_load(true);

		//Load image
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

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

	std::shared_future<void> Texture2D::Compile() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL ES on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!");

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

		//Return an empty future
		return {};
	}

	void Texture2D::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Try to invoke OpenGL ES and throw any exceptions back to the initial caller
			try {
				InvokeGL([this]() {
					this->Release();
				}).get();
				return;
			} catch(...) {
				std::rethrow_exception(std::current_exception());
			}
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled texture!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot release bound texture!");

		glDeleteTextures(1, &(nd->gpuID));
		compiled = false;
	}

	void Texture2D::Bind(int slot) {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot bind texture in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled texture!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound texture!");

		//Bind the texture to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, nd->gpuID);
		bound = true;
	}

	void Texture2D::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot unbind texture in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled texture!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound texture!");

		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		currentSlot = -1;
		bound = false;
	}
}