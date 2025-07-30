#pragma once

#include "impl/PAL.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/Exceptions.hpp"

#include <sstream>

#include "eternal.hpp"
#include "glad/gl.h"

constexpr const inline auto glErrors = mapbox::eternal::map<GLenum, mapbox::eternal::string>({{GL_INVALID_ENUM, "Invalid Enum"},
	{GL_INVALID_VALUE, "Invalid Value"},
	{GL_INVALID_OPERATION, "Invalid Operation"},
	{GL_INVALID_FRAMEBUFFER_OPERATION, "Invalid Framebuffer"},
	{GL_OUT_OF_MEMORY, "Out of Memory"},
	{GL_STACK_OVERFLOW, "Stack Overflow"},
	{GL_STACK_UNDERFLOW, "Stack Underflow"}});

#define GL_CHECK(msg)                                                                                                    \
	if(GLenum err = glGetError(); err != GL_NO_ERROR) {                                                                  \
		std::stringstream message;                                                                                       \
		message << "OpenGL error 0x" << std::hex << err << std::dec << " (" << glErrors.at(err).c_str() << "), " << msg; \
		Check<ExternalException>(false, message.str());                                                                  \
	}

namespace Cacao {
	class OpenGLModule : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;
		void Destroy() override;
		void SetVSync(bool state) override;

		//==================== IMPL POINTER CONFIGURATION ====================
		virtual Mesh::Impl* ConfigureMesh() override;
		virtual Tex2D::Impl* ConfigureTex2D() override;
		virtual Cubemap::Impl* ConfigureCubemap() override;

		OpenGLModule()
		  : PALModule("opengl") {}
		~OpenGLModule() {}

	  private:
		EventConsumer resizer;
	};

	inline std::shared_ptr<OpenGLModule> gl;
}