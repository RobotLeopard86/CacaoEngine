#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef Yield

#include <string>

#include "Cacao/Window.hpp"
#include "impl/Window.hpp"

namespace Cacao {
	class Win32WindowImpl : public Window::Impl {
	  public:
		void CreateWindow() override;
		void DestroyWindow() override;
		void HandleEvents() override;
		bool Minimized() override;
		const glm::uvec2 ContentAreaSize() override;
		void Visibility(bool visible) override;
		void Title(const std::string& title) override;
		void Resize(const glm::uvec2& size) override;
		void ModeChange(Window::Mode mode) override;
		void SaveWinPos() override;
		void SaveWinSize() override;
		void RestoreWin() override;

		unsigned int ConvertKeycode(unsigned int key) override;

		const std::string ProviderID() const override {
			return "win32";
		}

		HINSTANCE hInst;
		ATOM wndclass;
		HWND hWnd;
	};
}