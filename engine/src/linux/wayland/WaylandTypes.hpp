#pragma once

#include "Cacao/Window.hpp"
#include "impl/Window.hpp"

#include <wayland-client.h>
#include "libdecor.h"
#include "xdg-output-unstable-v1-client-protocol.h"

namespace Cacao {
	class WaylandWindowImpl : public Window::Impl {
	  public:
		wl_display* display = nullptr;
		wl_compositor* compositor = nullptr;
		wl_registry* registry = nullptr;
		wl_surface* surface = nullptr;
		zxdg_output_manager_v1* outMgr = nullptr;
		zxdg_output_v1* out = nullptr;
		wl_surface_listener surfListener = {};
		zxdg_output_v1_listener outListener = {};
		glm::ivec2 outputSize = {0, 0};
		libdecor* decor = nullptr;
		libdecor_frame* frame = nullptr;
		libdecor_interface decorInterface = {};
		libdecor_frame_interface frameInterface = {};
		bool configured = false;
		glm::uvec2 lastKnownContentSize;

		void CreateWindow() override;
		void DestroyWindow() override;
		void HandleEvents() override;

		//Wayland can't check for minimization
		bool Minimized() override {
			return false;
		}

		const glm::uvec2 ContentAreaSize() override;
		void Visibility(bool visible) override;
		void Title(const std::string& title) override;
		void Resize(const glm::uvec2& size) override;
		void ModeChange(Window::Mode mode) override;

		//Wayland can't see window position
		void SaveWinPos() override {}

		void SaveWinSize() override;
		void RestoreWin() override;

		const std::string ProviderID() const override {
			return "wayland";
		}
	};
}