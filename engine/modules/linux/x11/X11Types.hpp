#pragma once

#include "Cacao/Window.hpp"

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <string>
#include <vector>

namespace Cacao {
	class X11Common {
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

		void CreateWindow();
		void DestroyWindow();
		void HandleEvents();
		bool Minimized();
		const glm::uvec2 ContentAreaSize();
		void Visibility(bool visible);
		void Title(const std::string& title);
		void Resize(const glm::uvec2& size);
		void ModeChange(Window::Mode mode);
		void SaveWinPos();
		void SaveWinSize();
		void RestoreWin();
	};
}