#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "X11Types.hpp"

#include <memory>

#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>
#include "glm/gtc/type_ptr.hpp"

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
			XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE};
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

		//Send commands to X server and wait for completion
		xcb_flush(connection);

		//Connect graphics
		PAL::Get().GfxConnect();
	}

	void X11WindowImpl::DestroyWindow() {
		//Disconnect graphics
		PAL::Get().GfxDisconnect();

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

				//Window has gained focus
				case XCB_FOCUS_IN: {
					//Fire an event
					Event e("WindowFocus");
					EventManager::Get().Dispatch(e);
					break;
				}

				//Window has lost focus
				case XCB_FOCUS_OUT: {
					//Fire an event
					Event e("WindowUnfocus");
					EventManager::Get().Dispatch(e);
					break;
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
		//TODO
		return key;
	}
}