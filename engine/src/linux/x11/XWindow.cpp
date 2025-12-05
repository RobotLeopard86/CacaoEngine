#include "Cacao/Exceptions.hpp"
#include "Cacao/Input.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "X11Types.hpp"

#include <memory>

#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>

#include "glm/gtc/type_ptr.hpp"
#include "eternal.hpp"

#define win Window::Get()

namespace Cacao {
	struct XWinRegistrar {
		XWinRegistrar() {
			Window::Impl::registry.insert_or_assign("x11", []() { return std::make_unique<X11WindowImpl>(); });
		}
	};
	__attribute__((used)) XWinRegistrar xwr;

	void X11WindowImpl::CreateWindow() {
		//Connect to X server
		connection = xcb_connect(nullptr, nullptr);
		Check<ExternalException>(connection != nullptr, "Failed to connect to X server!");

		//Obtain screen (we do the first one, always)
		const xcb_setup_t* setup = xcb_get_setup(connection);
		xcb_screen_iterator_t scrIter = xcb_setup_roots_iterator(setup);
		xcb_screen_t* screen = scrIter.data;

		//We want to start the window centered, so time to calculate that position
		glm::i16vec2 centered = {(screen->width_in_pixels / 2) - (size.x / 2), (screen->height_in_pixels / 2) - (size.y / 2)};

		//Create window
		window = xcb_generate_id(connection);
		uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t valueList[] = {
			screen->black_pixel,
			XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
				XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION};
		xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, centered.x, centered.y, size.x,
			size.y, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, valueMask, valueList);

		//Set initial title
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, title.size(), title.c_str());

		//Allow the window to be closed (because X is stupid and doesn't assume that by default)
		//The top part is requesting the atoms to use for property setting, the last part is actually setting the property
		xcb_intern_atom_cookie_t protoCookie = xcb_intern_atom(connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* protoReply = xcb_intern_atom_reply(connection, protoCookie, nullptr);
		Check<ExternalException>(protoReply != nullptr, "Failed to query protocols atom from X server!");
		xcb_intern_atom_cookie_t delCookie = xcb_intern_atom(connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
		xcb_intern_atom_reply_t* delReply = xcb_intern_atom_reply(connection, delCookie, nullptr);
		Check<ExternalException>(delReply != nullptr, "Failed to query deletion atom from X server!");
		delAtom = delReply->atom;
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, protoReply->atom, XCB_ATOM_ATOM, 32, 1, &delAtom);
		free(protoReply);
		free(delReply);

		//If we should start visible, make the window visible
		if(visible) xcb_map_window(connection, window);

		//Allocate key symbols
		keysyms = xcb_key_symbols_alloc(connection);

		//Send commands to X server and wait for completion
		xcb_flush(connection);

		//Connect graphics
		PAL::Get().GfxConnect();
	}

	void X11WindowImpl::DestroyWindow() {
		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Free key symbols
		xcb_key_symbols_free(keysyms);

		//Destroy window
		xcb_destroy_window(connection, window);

		//Close connection
		xcb_disconnect(connection);

		//Reset values so we don't accidentally do some junk
		connection = nullptr;
		window = 0;
	}

	void X11WindowImpl::HandleEvents() {
		//We store this so we don't get flooded with intermediate configures and only respond to the last one
		xcb_configure_notify_event_t* lastCfgEvent = nullptr;

		//Process events
		xcb_generic_event_t* event;
		while((event = xcb_poll_for_event(connection)) != nullptr) {
			uint8_t responseType = event->response_type & ~0x80;
			switch(responseType) {
				//Window might be closing but we have to check
				case XCB_CLIENT_MESSAGE: {
					//Get the client message info
					xcb_client_message_event_t* cmEvent = reinterpret_cast<xcb_client_message_event_t*>(event);

					//If this isn't window close, break
					//Otherwise we allow fallthrough
					if(cmEvent->data.data32[0] != delAtom) break;
				}

				//Window is closing
				case XCB_DESTROY_NOTIFY:
					Engine::Get().Quit();
					break;

				//Window has been resized
				case XCB_CONFIGURE_NOTIFY: {
					//Ignore updates while in borderless mode
					if(mode == Window::Mode::Borderless) break;

					//Get the configure info
					xcb_configure_notify_event_t* cfgEvent = reinterpret_cast<xcb_configure_notify_event_t*>(event);

					//Set last configure event pointer
					if(lastCfgEvent) {
						free(lastCfgEvent);
					}
					lastCfgEvent = cfgEvent;
					break;
				}

				//Key pressed
				case XCB_KEY_PRESS: {
					//Get the key that was pressed
					xcb_key_press_event_t* keyEvent = reinterpret_cast<xcb_key_press_event_t*>(event);
					xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keyEvent->detail, 0);

					//Dispatch event
					DataEvent<unsigned int> event("KeyDown", ConvertKeycode(keysym));
					EventManager::Get().Dispatch(event);
				}

				//Key released
				case XCB_KEY_RELEASE: {
					//Get the key that was released
					xcb_key_press_event_t* keyEvent = reinterpret_cast<xcb_key_press_event_t*>(event);
					xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keyEvent->detail, 0);

					//Dispatch event
					DataEvent<unsigned int> event("KeyUp", ConvertKeycode(keysym));
					EventManager::Get().Dispatch(event);
				}

				//Mouse button pressed
				case XCB_BUTTON_PRESS: {
					//Get the button that was pressed
					xcb_button_press_event_t* buttonEvent = reinterpret_cast<xcb_button_press_event_t*>(event);

					//Dispatch event
					DataEvent<unsigned int> event("MousePress", ConvertButtonCode(buttonEvent->detail));
					EventManager::Get().Dispatch(event);
				}

				//Mouse button released
				case XCB_BUTTON_RELEASE: {
					//Get the button that was released
					xcb_button_press_event_t* buttonEvent = reinterpret_cast<xcb_button_press_event_t*>(event);

					//Dispatch event
					DataEvent<unsigned int> event("MouseRelease", ConvertButtonCode(buttonEvent->detail));
					EventManager::Get().Dispatch(event);
				}

				default: break;
			}

			//Free the event
			if(responseType != XCB_CONFIGURE_NOTIFY) free(event);
		}

		//Apply last configure event if needed
		if(lastCfgEvent) {
			//Apply the new size value to the window
			size = {lastCfgEvent->width, lastCfgEvent->height};

			//Fire an event
			DataEvent<glm::uvec2> wre("WindowResize", size);
			EventManager::Get().Dispatch(wre);
		}
	}

	bool X11WindowImpl::Minimized() {
		//Request the window state atom from the X server
		xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom(connection, 0, 8, "WM_STATE");
		xcb_intern_atom_reply_t* atomReply = xcb_intern_atom_reply(connection, atomCookie, nullptr);
		Check<ExternalException>(atomReply != nullptr, "Failed to query state atom from X server!");
		xcb_atom_t atom = atomReply->atom;
		free(atomReply);

		//Query the state property using the atom
		xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, window, atom, atom, 0, 2);
		xcb_get_property_reply_t* reply = xcb_get_property_reply(connection, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get state property from X server!");
		uint32_t* wmState = reinterpret_cast<uint32_t*>(xcb_get_property_value(reply));

		//At last, the value we want!
		bool iconified = (wmState[0] == XCB_ICCCM_WM_STATE_ICONIC);

		//Free property reply and return
		free(reply);
		return iconified;
	}

	//On X, content area size and window size are the same, and the returning method also has position info
	//A couple functions need this data, so this is a helper for transacting with the X server for it
	xcb_get_geometry_reply_t* FetchGeometry(xcb_connection_t* conn, xcb_window_t w) {
		xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn, w);
		xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(conn, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get window geometry from X server!");
		return reply;
	}

	const glm::uvec2 X11WindowImpl::ContentAreaSize() {
		//Request the window geometry from the X server
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		glm::uvec2 ret = {reply->width, reply->height};
		free(reply);
		return ret;
	}

	void X11WindowImpl::Visibility(bool visible) {
		if(visible) {
			xcb_map_window(connection, window);
		} else {
			xcb_unmap_window(connection, window);
		}
		xcb_flush(connection);
	}

	void X11WindowImpl::Title(const std::string& title) {
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, title.size(), title.c_str());
		xcb_flush(connection);
	}

	void X11WindowImpl::Resize(const glm::uvec2& size) {
		//Convert size to 16-bit for XCB so we can feed the GLM pointer
		glm::u16vec2 size16 {size.x, size.y};

		//Resize
		xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, glm::value_ptr(size16));
		xcb_flush(connection);
	}

	void X11WindowImpl::SaveWinPos() {
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		lastPos = {reply->x, reply->y};
		free(reply);
	}

	void X11WindowImpl::SaveWinSize() {
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		lastSize = {reply->width, reply->height};
		free(reply);
	}

	void X11WindowImpl::RestoreWin() {
		//Create value list
		const uint16_t valueList[] = {(uint16_t)lastPos.x, (uint16_t)lastPos.y, (uint16_t)lastSize.x, (uint16_t)lastSize.y};

		//Set size and position
		xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valueList);
		xcb_flush(connection);
	}

	void X11WindowImpl::ModeChange(Window::Mode mode) {
		//Fetch atoms

		//Request the window state atom from the X server
		xcb_intern_atom_cookie_t stateCookie = xcb_intern_atom(connection, 0, 8, "WM_STATE");
		xcb_intern_atom_reply_t* stateReply = xcb_intern_atom_reply(connection, stateCookie, nullptr);
		Check<ExternalException>(stateReply != nullptr, "Failed to query state atom from X server!");
		xcb_atom_t state = stateReply->atom;
		free(stateReply);

		//Request the fullscreen state atom from the X server
		xcb_intern_atom_cookie_t fullscreenCookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_STATE_FULLSCREEN"), "_NET_WM_STATE_FULLSCREEN");
		xcb_intern_atom_reply_t* fullscreenReply = xcb_intern_atom_reply(connection, fullscreenCookie, nullptr);
		Check<ExternalException>(fullscreenReply != nullptr, "Failed to query fullscreen state atom from X server!");
		xcb_atom_t fullscreen = fullscreenReply->atom;
		free(fullscreenReply);

		//Request the Motif hints atom from the X server
		xcb_intern_atom_cookie_t motifCookie = xcb_intern_atom(connection, 0, strlen("_MOTIF_WM_HINTS"), "_MOTIF_WM_HINTS");
		xcb_intern_atom_reply_t* motifReply = xcb_intern_atom_reply(connection, motifCookie, nullptr);
		Check<ExternalException>(motifReply != nullptr, "Failed to query Motif hints atom from X server!");
		xcb_atom_t motif = motifReply->atom;
		free(motifReply);

		//Request the compositor bypass atom from the X server
		xcb_intern_atom_cookie_t bypassCookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_BYPASS_COMPOSITOR"), "_NET_WM_BYPASS_COMPOSITOR");
		xcb_intern_atom_reply_t* bypassReply = xcb_intern_atom_reply(connection, bypassCookie, nullptr);
		Check<ExternalException>(bypassReply != nullptr, "Failed to query compositor bypass atom from X server!");
		xcb_atom_t bypass = bypassReply->atom;
		free(bypassReply);

		//Get RandR monitor

		//Get root window
		xcb_window_t rootWindow = 0;
		{
			const xcb_setup_t* setup = xcb_get_setup(connection);
			xcb_screen_iterator_t scrIter = xcb_setup_roots_iterator(setup);
			xcb_screen_t* screen = scrIter.data;
			rootWindow = screen->root;
		}

		//Get window absolute position
		xcb_translate_coordinates_cookie_t tcCookie = xcb_translate_coordinates(connection, window, rootWindow, 0, 0);
		xcb_translate_coordinates_reply_t* tcReply = xcb_translate_coordinates_reply(connection, tcCookie, nullptr);
		Check<ExternalException>(tcReply != nullptr, "Failed to translate window coordinates!");
		glm::ivec2 winAbsPos = {tcReply->dst_x, tcReply->dst_y};
		free(tcReply);

		//Find all the monitors
		xcb_randr_get_monitors_cookie_t monitorGetCookie = xcb_randr_get_monitors(connection, window, true);
		xcb_randr_get_monitors_reply_t* monitorGetReply = xcb_randr_get_monitors_reply(connection, monitorGetCookie, nullptr);
		Check<ExternalException>(monitorGetReply != nullptr, "Failed to find monitors!");
		xcb_randr_monitor_info_iterator_t monInfo = xcb_randr_get_monitors_monitors_iterator(monitorGetReply);
		int monCount = xcb_randr_get_monitors_monitors_length(monitorGetReply);

		//Find the monitor which the window is likely on
		xcb_randr_monitor_info_t monitor = {};
		bool found = false;
		for(int i = 0; i < monCount; ++i) {
			xcb_randr_monitor_info_t mon = monInfo.data[i];
			if(winAbsPos.x >= mon.x && winAbsPos.x < (mon.x + mon.width) && winAbsPos.y >= mon.y && winAbsPos.y < (mon.y + mon.height)) {
				monitor = mon;
				found = true;
				free(monitorGetReply);
				break;
			}
		}
		if(!found) free(monitorGetReply);
		Check<NonexistentValueException>(found, "Failed to find window monitor!");
		xcb_randr_output_t output = xcb_randr_monitor_info_outputs(&monitor)[0];

		//Remove fullscreen state and restore CRTC config if needed
		if(mode != Window::Mode::Fullscreen && this->mode == Window::Mode::Fullscreen) {
			xcb_delete_property(connection, window, state);
			if(crtcSet) xcb_randr_set_crtc_config(connection, crtcState.crtc, crtcState.timestamp, crtcState.configTimestamp, crtcState.position.x,
				crtcState.position.y, crtcState.videoMode, crtcState.rotation, crtcState.outputs.size(), crtcState.outputs.data());
		}

		//Define Motif hints (for decorations)
		struct MotifHints {
			uint32_t flags = 2;
			uint32_t functions = 0;
			uint32_t decorations = 0;
			int32_t input_mode = 0;
			uint32_t status = 0;
		} hints;
		uint32_t doBypass = 0;

		//Actually make the change
		switch(mode) {
			case Window::Mode::Windowed:
				//Enable decorations
				hints.flags = 1;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, motif, XCB_ATOM_ATOM, 32, 5, &hints);

				//Disable compositor bypass
				doBypass = 0;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, bypass, XCB_ATOM_CARDINAL, 32, 1, &doBypass);
				break;
			case Window::Mode::Borderless: {
				//Disable decorations
				hints.flags = 0;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, motif, XCB_ATOM_ATOM, 32, 5, &hints);

				//Enable compositor bypass
				doBypass = 1;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, bypass, XCB_ATOM_CARDINAL, 32, 1, &doBypass);

				//Set window size to monitor size
				int16_t valueList[] = {monitor.x, monitor.y, (int16_t)monitor.width, (int16_t)monitor.height};
				xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valueList);
				size = {monitor.width, monitor.height};

				//Set the fullscreen property
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, state, XCB_ATOM_ATOM, 32, 1, &fullscreen);
				break;
			}
			case Window::Mode::Fullscreen: {
				//Get screen resources and video modes
				xcb_randr_get_screen_resources_current_cookie_t resCookie = xcb_randr_get_screen_resources_current(connection, window);
				xcb_randr_get_screen_resources_current_reply_t* resReply = xcb_randr_get_screen_resources_current_reply(connection, resCookie, nullptr);
				Check<ExternalException>(resReply != nullptr, "Failed to get screen resources!");
				xcb_randr_mode_info_t* modes = xcb_randr_get_screen_resources_current_modes(resReply);
				int modeCount = xcb_randr_get_screen_resources_current_modes_length(resReply);

				//Select best video mode (may not be exact)
				xcb_randr_mode_t bestVideoMode = 0;
				unsigned int xDistLast = UINT32_MAX, yDistLast = UINT32_MAX;
				bool foundExact = false;
				glm::u16vec2 vidModeSize = {0, 0};
				for(int i = 0; i < modeCount; ++i) {
					xcb_randr_mode_info_t modeInfo = modes[i];

					//If we found it exactly, we're golden
					if(modeInfo.width == size.x && modeInfo.height == size.y) {
						bestVideoMode = modeInfo.id;
						foundExact = true;
						break;
					}

					//Otherwise, keep selecting modes that are closest to the resolution until we find one
					unsigned int xDistCurrent = abs((int)size.x - modeInfo.width);
					unsigned int yDistCurrent = abs((int)size.y - modeInfo.height);
					if(xDistCurrent < xDistLast || yDistCurrent < yDistLast) {
						bestVideoMode = modeInfo.id;
						xDistLast = xDistCurrent;
						yDistLast = yDistCurrent;
						vidModeSize = {modeInfo.width, modeInfo.height};
					}
				}
				free(resReply);

				//Set size to best video mode's if not exact
				if(!foundExact) size = vidModeSize;

				//Find ouput CRTC
				xcb_randr_get_output_info_cookie_t outInfoCookie = xcb_randr_get_output_info(connection, output, XCB_CURRENT_TIME);
				xcb_randr_get_output_info_reply_t* outInfoReply = xcb_randr_get_output_info_reply(connection, outInfoCookie, nullptr);
				Check<ExternalException>(outInfoReply != nullptr, "Failed to get output info!");
				Check<NonexistentValueException>(outInfoReply->crtc != XCB_NONE, "Current output has no CRTC!");
				xcb_randr_crtc_t crtc = outInfoReply->crtc;
				free(outInfoReply);

				//Save CRTC config
				xcb_randr_get_crtc_info_cookie_t crtcCookie = xcb_randr_get_crtc_info(connection, crtc, XCB_CURRENT_TIME);
				xcb_randr_get_crtc_info_reply_t* crtcReply = xcb_randr_get_crtc_info_reply(connection, crtcCookie, nullptr);
				Check<ExternalException>(crtcReply != nullptr, "Failed to get output CRTC info!");
				crtcSet = true;
				crtcState.crtc = crtc;
				crtcState.timestamp = crtcReply->timestamp;
				crtcState.configTimestamp = crtcReply->timestamp;
				crtcState.videoMode = crtcReply->mode;
				crtcState.position.x = crtcReply->x;
				crtcState.position.y = crtcReply->y;
				crtcState.size.x = crtcReply->width;
				crtcState.size.y = crtcReply->height;
				crtcState.rotation = crtcReply->rotation;
				crtcState.outputs.assign(xcb_randr_get_crtc_info_outputs(crtcReply), xcb_randr_get_crtc_info_outputs(crtcReply) + (xcb_randr_get_crtc_info_outputs_length(crtcReply) * sizeof(xcb_randr_output_t)));

				//Disable decorations
				hints.flags = 0;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, motif, XCB_ATOM_ATOM, 32, 5, &hints);

				//Enable compositor bypass
				doBypass = 1;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, bypass, XCB_ATOM_CARDINAL, 32, 1, &doBypass);

				//Set window size to video mode size
				int16_t valueList[] = {monitor.x, monitor.y, (int16_t)size.x, (int16_t)size.y};
				xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valueList);

				//Set new CRTC config
				xcb_randr_set_crtc_config(connection, crtc, XCB_CURRENT_TIME, XCB_CURRENT_TIME, (uint32_t)monitor.x, (uint32_t)monitor.y, bestVideoMode, XCB_RANDR_ROTATION_ROTATE_0, 1, &output);

				break;
			}
		}

		//Send any unprocessed commands to the X server and wait
		xcb_flush(connection);
	}

	unsigned int X11WindowImpl::ConvertKeycode(unsigned int key) {
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({{XK_Return, CACAO_KEY_ENTER},
			{XK_Escape, CACAO_KEY_ESCAPE},
			{XK_BackSpace, CACAO_KEY_BACKSPACE},
			{XK_Tab, CACAO_KEY_TAB},
			{XK_space, CACAO_KEY_SPACE},
			{XK_apostrophe, CACAO_KEY_APOSTROPHE},
			{XK_comma, CACAO_KEY_COMMA},
			{XK_minus, CACAO_KEY_MINUS},
			{XK_equal, CACAO_KEY_EQUALS},
			{XK_period, CACAO_KEY_PERIOD},
			{XK_slash, CACAO_KEY_SLASH},
			{XK_0, CACAO_KEY_0},
			{XK_1, CACAO_KEY_1},
			{XK_2, CACAO_KEY_2},
			{XK_3, CACAO_KEY_3},
			{XK_4, CACAO_KEY_4},
			{XK_5, CACAO_KEY_5},
			{XK_6, CACAO_KEY_6},
			{XK_7, CACAO_KEY_7},
			{XK_8, CACAO_KEY_8},
			{XK_9, CACAO_KEY_9},
			{XK_semicolon, CACAO_KEY_SEMICOLON},
			{XK_bracketleft, CACAO_KEY_LEFT_BRACKET},
			{XK_bracketright, CACAO_KEY_RIGHT_BRACKET},
			{XK_backslash, CACAO_KEY_BACKSLASH},
			{XK_grave, CACAO_KEY_GRAVE_ACCENT},
			{XK_a, CACAO_KEY_A},
			{XK_b, CACAO_KEY_B},
			{XK_c, CACAO_KEY_C},
			{XK_d, CACAO_KEY_D},
			{XK_e, CACAO_KEY_E},
			{XK_f, CACAO_KEY_F},
			{XK_g, CACAO_KEY_G},
			{XK_h, CACAO_KEY_H},
			{XK_i, CACAO_KEY_I},
			{XK_j, CACAO_KEY_J},
			{XK_k, CACAO_KEY_K},
			{XK_l, CACAO_KEY_L},
			{XK_m, CACAO_KEY_M},
			{XK_n, CACAO_KEY_N},
			{XK_o, CACAO_KEY_O},
			{XK_p, CACAO_KEY_P},
			{XK_q, CACAO_KEY_Q},
			{XK_r, CACAO_KEY_R},
			{XK_s, CACAO_KEY_S},
			{XK_t, CACAO_KEY_T},
			{XK_u, CACAO_KEY_U},
			{XK_v, CACAO_KEY_V},
			{XK_w, CACAO_KEY_W},
			{XK_x, CACAO_KEY_X},
			{XK_y, CACAO_KEY_Y},
			{XK_z, CACAO_KEY_Z},
			{XK_Caps_Lock, CACAO_KEY_CAPS_LOCK},
			{XK_F1, CACAO_KEY_F1},
			{XK_F2, CACAO_KEY_F2},
			{XK_F3, CACAO_KEY_F3},
			{XK_F4, CACAO_KEY_F4},
			{XK_F5, CACAO_KEY_F5},
			{XK_F6, CACAO_KEY_F6},
			{XK_F7, CACAO_KEY_F7},
			{XK_F8, CACAO_KEY_F8},
			{XK_F9, CACAO_KEY_F9},
			{XK_F10, CACAO_KEY_F10},
			{XK_F11, CACAO_KEY_F11},
			{XK_F12, CACAO_KEY_F12},
			{XK_Print, CACAO_KEY_PRINT_SCREEN},
			{XK_Scroll_Lock, CACAO_KEY_SCROLL_LOCK},
			{XK_Pause, CACAO_KEY_PAUSE},
			{XK_Insert, CACAO_KEY_INSERT},
			{XK_Delete, CACAO_KEY_DELETE},
			{XK_Home, CACAO_KEY_HOME},
			{XK_Page_Up, CACAO_KEY_PAGE_UP},
			{XK_End, CACAO_KEY_END},
			{XK_Page_Down, CACAO_KEY_PAGE_DOWN},
			{XK_Right, CACAO_KEY_RIGHT},
			{XK_Left, CACAO_KEY_LEFT},
			{XK_Down, CACAO_KEY_DOWN},
			{XK_Up, CACAO_KEY_UP},
			{XK_Num_Lock, CACAO_KEY_NUM_LOCK},
			{XK_KP_Divide, CACAO_KEY_KP_DIVIDE},
			{XK_KP_Multiply, CACAO_KEY_KP_MULTIPLY},
			{XK_KP_Subtract, CACAO_KEY_KP_MINUS},
			{XK_KP_Add, CACAO_KEY_KP_PLUS},
			{XK_KP_Enter, CACAO_KEY_KP_ENTER},
			{XK_KP_1, CACAO_KEY_KP_1},
			{XK_KP_2, CACAO_KEY_KP_2},
			{XK_KP_3, CACAO_KEY_KP_3},
			{XK_KP_4, CACAO_KEY_KP_4},
			{XK_KP_5, CACAO_KEY_KP_5},
			{XK_KP_6, CACAO_KEY_KP_6},
			{XK_KP_7, CACAO_KEY_KP_7},
			{XK_KP_8, CACAO_KEY_KP_8},
			{XK_KP_9, CACAO_KEY_KP_9},
			{XK_KP_0, CACAO_KEY_KP_0},
			{XK_KP_Decimal, CACAO_KEY_KP_PERIOD},
			{XK_Control_L, CACAO_KEY_LEFT_CONTROL},
			{XK_Shift_L, CACAO_KEY_LEFT_SHIFT},
			{XK_Alt_L, CACAO_KEY_LEFT_ALT},
			{XK_Super_L, CACAO_KEY_LEFT_SUPER},
			{XK_Control_R, CACAO_KEY_RIGHT_CONTROL},
			{XK_Shift_R, CACAO_KEY_RIGHT_SHIFT},
			{XK_Alt_R, CACAO_KEY_RIGHT_ALT},
			{XK_Super_R, CACAO_KEY_RIGHT_SUPER}});
		if(codes.contains(key)) return codes.at(key);
		return UINT32_MAX;
	}

	unsigned int X11WindowImpl::ConvertButtonCode(unsigned int button) {
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({{1, CACAO_BUTTON_LEFT},
			{2, CACAO_BUTTON_MIDDLE},
			{3, CACAO_BUTTON_RIGHT}});
		if(codes.contains(button)) return codes.at(button);
		return UINT32_MAX;
	}
}