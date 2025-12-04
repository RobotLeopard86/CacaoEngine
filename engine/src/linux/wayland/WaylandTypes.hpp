#pragma once

#include "Cacao/Window.hpp"
#include "impl/Window.hpp"

#include "libdecor.h"
#include "xdg-output-unstable-v1-client-protocol.h"

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

namespace Cacao {
	class WaylandWindowImpl : public Window::Impl {
	  public:
		//Core Wayland objects
		wl_display* display = nullptr;
		wl_compositor* compositor = nullptr;
		wl_registry* registry = nullptr;
		wl_surface* surface = nullptr;

		//xdg_output stuff
		zxdg_output_manager_v1* outMgr = nullptr;
		zxdg_output_v1* out = nullptr;

		//Sizes
		glm::ivec2 outputSize = {0, 0};
		glm::uvec2 lastKnownContentSize;

		//libdecor stuff
		libdecor* decor = nullptr;
		libdecor_frame* frame = nullptr;
		libdecor_interface decorInterface = {};
		libdecor_frame_interface frameInterface = {};
		bool configured = false;

		//Input stuff
		wl_seat* seat = nullptr;
		wl_keyboard* keyboard = nullptr;
		wl_pointer* mouse = nullptr;
		xkb_context* xkb = nullptr;
		xkb_keymap* keymap = nullptr;
		xkb_state* xkbState = nullptr;

		//Event listeners
		wl_surface_listener surfListener = {};
		zxdg_output_v1_listener outListener = {};
		wl_keyboard_listener keyboardListener = {};
		wl_pointer_listener mouseListener = {};

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

		unsigned int ConvertKeycode(unsigned int key) override;
		unsigned int ConvertButtonCode(unsigned int button) override;

		const std::string ProviderID() const override {
			return "wayland";
		}
	};
}