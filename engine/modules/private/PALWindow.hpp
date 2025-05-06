#pragma once

#include "PALCommon.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"

namespace Cacao {
	class CACAO_API PALWindowInterface : public PALInterface {
	  public:
		virtual void CreateNativeWindow() = 0;
		virtual void DestroyNativeWindow() = 0;
		virtual void* GetNativeWindow() = 0;
		virtual void ApplyVisibility() = 0;
		virtual void ApplyMode() = 0;
		virtual void ApplySize() = 0;
		virtual void ApplyTitle() = 0;
		virtual glm::uvec2 GetContentAreaSize() = 0;

		virtual ~PALWindowInterface() {}

	  protected:
		PALWindowInterface(Window* w)
		  : win(w) {}

		Window* win;
	};
}