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

		OpenGLModule()
		  : PALModule("opengl") {}
		~OpenGLModule() {}

	  private:
		EventConsumer resizer;
	};

	inline std::shared_ptr<OpenGLModule> gl;
}