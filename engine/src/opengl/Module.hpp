#pragma once

#include "PALCommon.hpp"
#include "Cacao/EventSystem.hpp"

namespace Cacao {
	class OpenGLModule : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;
		void Destroy() override;
		void SetVSync(bool state) override;

		/* ------------------------------------------- *\
		|*      PLACEHOLDER: IMPL CONFIGURATORS        *|
		\* ------------------------------------------- */

		OpenGLModule()
		  : PALModule("opengl") {}
		~OpenGLModule() {}

	  private:
		EventConsumer resizer;
		
	};

	inline std::shared_ptr<OpenGLModule> gl;
}