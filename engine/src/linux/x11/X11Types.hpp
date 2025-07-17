#pragma once

#include "Cacao/Window.hpp"
#include "impl/Window.hpp"

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <string>
#include <vector>

namespace Cacao {
	class X11WindowImpl : public Window::Impl {
	  public:
		xcb_connection_t* connection;
		xcb_window_t window;

		struct CRTCState {
			xcb_randr_crtc_t crtc;
			xcb_timestamp_t timestamp;
			xcb_timestamp_t configTimestamp;
			xcb_randr_mode_t videoMode;
			glm::i16vec2 position;
			glm::u16vec2 size;
			uint16_t rotation;
			std::vector<xcb_randr_output_t> outputs;
		} crtcState;
		bool crtcSet;

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

		const std::string ProviderID() const override {
			return "x11";
		}
	};
}