#pragma once

#include "impl/PAL.hpp"
#include "Cacao/EventConsumer.hpp"

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