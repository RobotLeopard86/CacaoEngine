#include "OpenGLTex2D.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

#include "glad/gl.h"

#include "libcacaoimage.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> OpenGLTex2DImpl::Realize(bool& success) {
		//Open-GL specific stuff needs to be on the main thread
		return Engine::Get().RunTaskOnMainThread([this, &success]() {
			//Get texture format
			GLenum internalFormat;
			switch(img.layout) {
				case libcacaoimage::Image::Layout::Grayscale:
					format = GL_RED;
					internalFormat = GL_RED;
					break;
				case libcacaoimage::Image::Layout::RGB:
					format = GL_RGB;
					internalFormat = GL_SRGB8;
					break;
				case libcacaoimage::Image::Layout::RGBA:
					format = GL_RGBA;
					internalFormat = GL_SRGB8_ALPHA8;
					break;
			}

			//Create texture object
			glGenTextures(1, &gpuTex);
			GL_CHECK("Failed to create texture object!")

			//Bind texture
			glBindTexture(GL_TEXTURE_2D, gpuTex);

			//Transfer image data
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.w, img.h, 0, format, GL_UNSIGNED_BYTE, img.data.data());
			GL_CHECK("Failed to transfer image data to GPU texture!")

			//Configure and generate mipmaps
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
			glGenerateMipmap(GL_TEXTURE_2D);

			//Set texture wrapping mode
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			//Unbind texture
			glBindTexture(GL_TEXTURE_2D, 0);

			success = true;
		});
	}

	void OpenGLTex2DImpl::DropRealized() {
		Engine::Get().RunTaskOnMainThread([this]() {
			//Destroy texture object
			glDeleteTextures(1, &gpuTex);
		});
	}

	Tex2D::Impl* OpenGLModule::ConfigureTex2D() {
		return new OpenGLTex2DImpl();
	}
}