#pragma once

#include "Cacao/Window.hpp"
#include "Cacao/Sound.hpp"

#define IMPL(tp, ...) ImplAccessor::Get().Get##tp(__VA_ARGS__)

namespace Cacao {
	class ImplAccessor {
	  public:
		static ImplAccessor& Get();

		ImplAccessor(const ImplAccessor&) = delete;
		ImplAccessor(ImplAccessor&&) = delete;
		ImplAccessor& operator=(const ImplAccessor&) = delete;
		ImplAccessor& operator=(ImplAccessor&&) = delete;

		Window::Impl& GetWindow() {
			return *Window::Get().impl;
		}

		Sound::Impl& GetSound(Sound& s) {
			return *s.impl;
		}

	  private:
		ImplAccessor();
	};
}