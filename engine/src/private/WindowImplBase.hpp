#pragma once

#include "Cacao/Window.hpp"

#include <map>
#include <functional>

namespace Cacao {
	class Window::Impl {
	  public:
		virtual void CreateWindow() = 0;
		virtual void DestroyWindow() = 0;
		virtual void HandleEvents() = 0;
		virtual bool Minimized() = 0;
		virtual const glm::uvec2 ContentAreaSize() = 0;
		virtual void Visibility(bool visible) = 0;
		virtual void Title(const std::string& title) = 0;
		virtual void Resize(const glm::uvec2& size) = 0;
		virtual void ModeChange(Window::Mode mode) = 0;
		virtual void SaveWinPos() = 0;
		virtual void SaveWinSize() = 0;
		virtual void RestoreWin() = 0;

		virtual ~Impl() = default;

		bool open, visible;
		Mode mode;
		glm::uvec2 size;
		std::string title;
		glm::uvec2 lastPos;
		glm::uvec2 lastSize;

		// clang-format off
		static std::map<std::string, std::function<std::unique_ptr<Impl>&&()>> registry;
		// clang-format on
	};
}