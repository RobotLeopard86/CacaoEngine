#include "OpenGLCubemap.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"

#include "glad/gl.h"
#include "libcacaoimage.hpp"

namespace Cacao {
	void OpenGLCubemapImpl::Realize(bool& success) {
		//Open-GL specific stuff needs to be on the GPU thread
		OpenGLCommandBuffer cmd([this, &success]() {
			//Create texture object
			glGenTextures(1, &gpuTex);
			GL_CHECK("Failed to create cubemap texture object!")

			//Bind texture
			glBindTexture(GL_TEXTURE_CUBE_MAP, gpuTex);

			//Transfer face images
			uint8_t i = 0;
			for(const libcacaoimage::Image& image : faces) {
				//Flip image because OpenGL likes it that way
				libcacaoimage::Image flipped = libcacaoimage::Flip(image);

				//Copy image data to GPU and adjust increment
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++, 0, GL_SRGB8, flipped.w, flipped.h, 0, GL_RGB, GL_UNSIGNED_BYTE, flipped.data.data());
				GL_CHECK("Failed to upload cubemap face texture data!")
			}

			//Apply cubemap filtering
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//Configure wrapping mode
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			//Unbind texture
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			success = true;
		});
		GPUManager::Get().Submit(std::move(cmd)).get();
	}

	void OpenGLCubemapImpl::DropRealized() {
		OpenGLCommandBuffer cmd([this]() {
			//Destroy texture object
			glDeleteTextures(1, &gpuTex);

			//Zero object name to avoid confusion
			gpuTex = 0;
		});
		GPUManager::Get().Submit(std::move(cmd)).get();
	}

	Cubemap::Impl* OpenGLModule::ConfigureCubemap() {
		return new OpenGLCubemapImpl();
	}
}