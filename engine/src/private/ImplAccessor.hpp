#pragma once

#include "Cacao/Window.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/PAL.hpp"

#define IMPL(tp, ...) ImplAccessor::Get().Get##tp(__VA_ARGS__)
#define WIN_IMPL(tp) static_cast<tp##WindowImpl&>(ImplAccessor::Get().GetWindow())

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

		ResourceManager::Impl& GetResourceManager() {
			return *ResourceManager::Get().impl;
		}

		PAL::Impl& GetPAL() {
			return *PAL::Get().impl;
		}

		Sound::Impl& GetSound(Sound& s) {
			return *s.impl;
		}

	  private:
		ImplAccessor();
	};
}