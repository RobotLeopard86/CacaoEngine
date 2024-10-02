#pragma once

#include "Events/EventSystem.hpp"
#include "Utilities/Task.hpp"
#include "GLHeaders.hpp"

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

	inline GLuint globalsUBO = 37;

	inline GLuint LinearFormat2Srgb(GLuint in) {
		switch(in) {
			case GL_RED: return GL_RED;
			case GL_RGB: return GL_SRGB;
			case GL_RGBA: return GL_SRGB_ALPHA;
			case GL_RGB8: return GL_SRGB8;
			case GL_RGBA8: return GL_SRGB8_ALPHA8;
			default: return 0;
		}
	}

	inline GLuint SrgbFormat2Linear(GLuint in) {
		switch(in) {
			case GL_RED: return GL_RED;
			case GL_SRGB: return GL_RGB;
			case GL_SRGB_ALPHA: return GL_RGBA;
			case GL_SRGB8: return GL_RGB8;
			case GL_SRGB8_ALPHA8: return GL_RGBA8;
			default: return 0;
		}
	}
}