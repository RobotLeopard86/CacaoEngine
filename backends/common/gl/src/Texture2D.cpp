#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "GLTexture2DData.hpp"
#include "GLUtils.hpp"

#include "stb_image.h"

#include "glad/gl.h"

#include <future>
#include <filesystem>

namespace Cacao {
	Texture2D::Texture2D(std::string filePath)
	  : Texture(false) {
		//Create native data
		nativeData.reset(new Tex2DData());

		//Ensure stb_image flips image Y (because OpenGL)
		stbi_set_flip_vertically_on_load(true);

		//Load image
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

		CheckException(dataBuffer, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load 2D texture image file!");

		bound = false;
		currentSlot = -1;

		//Determine image format
		if(numImgChannels == 1) {
			nativeData->format = GL_RED;
		} else if(numImgChannels == 3) {
			nativeData->format = GL_SRGB8;
		} else if(numImgChannels == 4) {
			nativeData->format = GL_SRGB8_ALPHA8;
		}
	}

	std::shared_future<void> Texture2D::Compile() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!");

		//Create texture object
		glGenTextures(1, &(nativeData->gpuID));

		//Bind texture object so we can work on it
		glBindTexture(GL_TEXTURE_2D, nativeData->gpuID);

		//Load image data into texture object
		glTexImage2D(GL_TEXTURE_2D, 0, nativeData->format, imgSize.x, imgSize.y, 0, GetTextureMemoryFormat(nativeData->format), GL_UNSIGNED_BYTE, dataBuffer);

		//Apply texture mipmap filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Configure mipmap levels
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);

		//Generate mipmaps for texture
		glGenerateMipmap(GL_TEXTURE_2D);

		//Configure texture wrapping mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Unbind texture object since we're done with it for now
		glBindTexture(GL_TEXTURE_2D, 0);

		compiled = true;

		//Return an empty future
		return {};
	}

	void Texture2D::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Try to invoke OpenGL and throw any exceptions back to the initial caller
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

		glDeleteTextures(1, &(nativeData->gpuID));
		compiled = false;
	}

	void Texture2D::Bind(int slot) {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot bind texture in non-rendering thread!");
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled texture!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound texture!");

		//Bind the texture to the requested slot
		currentSlot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, nativeData->gpuID);
		bound = true;
	}

	void Texture2D::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot unbind texture in non-rendering thread!");
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled texture!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound texture!");

		//Unbind the texture from its current slot
		glActiveTexture(GL_TEXTURE0 + currentSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		currentSlot = -1;
		bound = false;
	}
}