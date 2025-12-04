#pragma once

#include "Cacao/Window.hpp"

#include <unordered_map>
#include <functional>

#ifdef _WIN32
#undef CreateWindow
#undef DestroyWindow
#endif

namespace Cacao {
	class Window::Impl {
	  public:
		//======================= WINDOW FUNCTIONALITY =======================
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
		virtual const std::string ProviderID() const = 0;
		virtual unsigned int ConvertKeycode(unsigned int key) = 0;
		virtual unsigned int ConvertButtonCode(unsigned int button) = 0;

		virtual ~Impl() = default;

		//======================= STATE PROPERTIES =======================
		bool open, visible;
		Mode mode;
		glm::uvec2 size;
		std::string title;
		glm::uvec2 lastPos;
		glm::uvec2 lastSize;

		//Scroll data accumulation
		//Most OSes send a lot of intermediate events for one scroll
		glm::dvec2 scrollAccumulator = {0, 0};

		// clang-format off
		static std::unordered_map<std::string, std::function<std::unique_ptr<Impl>()>> registry;
		// clang-format on
	};
}