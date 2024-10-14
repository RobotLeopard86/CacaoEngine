#pragma once

#include "Events/EventSystem.hpp"
#include "Utilities/Task.hpp"
#include "glad/gl.h"

#include <future>
#include <memory>

namespace Cacao {
	void EnqueueGLJob(Task& task);

	inline std::shared_future<void> InvokeGL(std::function<void()> job) {
		Task glJob(job);
		EnqueueGLJob(glJob);
		return glJob.status->get_future().share();
	}

	struct RawGLTexture {
		GLuint texObj;
		int* slot;
	};

	inline GLuint globalsUBO = 0;

	inline GLenum GetTextureMemoryFormat(GLenum internalFormat) {
		switch(internalFormat) {
			case GL_RED: return GL_RED;
			case GL_RGB8: return GL_RGB;
			case GL_RGBA8: return GL_RGBA;
			case GL_SRGB8: return GL_RGB;
			case GL_SRGB8_ALPHA8: return GL_RGBA;
			default: return 0;
		}
	}
}
