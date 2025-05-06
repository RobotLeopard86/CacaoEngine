#pragma once

#include "PALCommon.hpp"
#include "Cacao/Window.hpp"

namespace Cacao {
	class CACAO_API PALWindowInterface : public PALInterface {
	  public:
		void CreateNativeWindow();
		void DestroyNativeWindow();
		void* GetNativeWindow();
		void ApplyVisibility();
		void ApplyMode();
		void ApplySize();
		void ApplyTitle();
		glm::uvec2 GetContentAreaSize();

	  protected:
		PALWindowInterface(Window* w)
		  : win(w) {}

		Window* win;
	};
}