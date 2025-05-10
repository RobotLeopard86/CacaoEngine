#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "X11Types.hpp"
#include "../LinuxRouter.hpp"

#include <memory>

#include <xcb/xcb_icccm.h>
#include <xcb/randr.h>
#include "glm/gtc/type_ptr.hpp"

#define win Window::Get()

namespace Cacao {
	void X11Common::CreateWindow() {
		//Connect to X server
		connection = xcb_connect(nullptr, nullptr);
		Check<ExternalException>(connection != nullptr, "Failed to connect to X server!");

		//Obtain screen (we do the first one, always)
		const xcb_setup_t* setup = xcb_get_setup(connection);
		xcb_screen_iterator_t scrIter = xcb_setup_roots_iterator(setup);
		xcb_screen_t* screen = scrIter.data;

		//We want to start the window centered, so time to calculate that position
		glm::i16vec2 centered = {(screen->width_in_pixels / 2) - (win.size.x / 2), (screen->height_in_pixels / 2) - (win.size.y / 2)};

		//Create window
		window = xcb_generate_id(connection);
		uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t valueList[] = {
			screen->black_pixel,
			XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_KEY_PRESS};
		xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, centered.x, centered.y, win.size.x,
			win.size.y, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, valueMask, valueList);

		//Set initial title
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, win.title.size(), win.title.c_str());

		//Allow the window to be closed (because X is stupid and doesn't assume that by default)
		//The top part is requesting the atoms to use for property setting, the last part is actually setting the property
		xcb_intern_atom_cookie_t protoCookie = xcb_intern_atom(connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* protoReply = xcb_intern_atom_reply(connection, protoCookie, nullptr);
		Check<ExternalException>(protoReply != nullptr, "Failed to query protocols atom from X server!");
		xcb_intern_atom_cookie_t delCookie = xcb_intern_atom(connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
		xcb_intern_atom_reply_t* delReply = xcb_intern_atom_reply(connection, delCookie, nullptr);
		Check<ExternalException>(delReply != nullptr, "Failed to query deletion atom from X server!");
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, protoReply->atom, XCB_ATOM_ATOM, 32, 1, &delReply->atom);
		free(protoReply);
		free(delReply);

		//If we should start visible, make the window visible
		if(win.visible) xcb_map_window(connection, window);

		//Send commands to X server and wait for completion
		xcb_flush(connection);
	}

	void X11Common::DestroyWindow() {
		//Destroy window
		xcb_destroy_window(connection, window);

		//Close connection
		xcb_disconnect(connection);

		//Reset values so we don't accidentally do some junk
		connection = nullptr;
		screen = nullptr;
		window = 0;
	}

	void X11Common::HandleEvents() {
		xcb_generic_event_t* event;
		while((event = xcb_poll_for_event(connection)) != nullptr) {
			//Handle window close
			uint8_t response_type = event->response_type & ~0x80;
			if(response_type == XCB_CLIENT_MESSAGE || response_type == XCB_DESTROY_NOTIFY) {
				Engine::Get().Quit();
			}

			//Free the event
			free(event);
		}
	}

	bool X11Common::Minimized() {
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

	const glm::uvec2 X11Common::ContentAreaSize() {
		//Request the window geometry from the X server
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		glm::uvec2 ret = {reply->width, reply->height};
		free(reply);
		return ret;
	}

	void X11Common::Visibility(bool visible) {
		if(visible) {
			xcb_map_window(connection, window);
		} else {
			xcb_unmap_window(connection, window);
		}
		xcb_flush(connection);
	}

	void X11Common::Title(const std::string& title) {
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, title.size(), title.c_str());
		xcb_flush(connection);
	}

	void X11Common::Resize(const glm::uvec2& size) {
		//Convert size to 16-bit for XCB so we can feed the GLM pointer
		glm::u16vec2 size16 {size.x, size.y};

		//Resize
		xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, glm::value_ptr(size16));
		xcb_flush(connection);
	}

	void X11Common::SaveWinPos() {
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		win.lastPos = {reply->x, reply->y};
		free(reply);
	}

	void X11Common::SaveWinSize() {
		xcb_get_geometry_reply_t* reply = FetchGeometry(connection, window);
		win.lastSize = {reply->width, reply->height};
		free(reply);
	}

	void X11Common::RestoreWin() {
		//Create value list
		const uint16_t valueList[] = {(uint16_t)win.lastPos.x, (uint16_t)win.lastPos.y, (uint16_t)win.lastSize.x, (uint16_t)win.lastSize.y};

		//Set size and position
		xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valueList);
		xcb_flush(connection);
	}

	void X11Common::ModeChange(Window::Mode mode) {
		//Request the window state atom from the X server
		xcb_intern_atom_cookie_t stateCookie = xcb_intern_atom(connection, 0, 8, "WM_STATE");
		xcb_intern_atom_reply_t* stateReply = xcb_intern_atom_reply(connection, stateCookie, nullptr);
		Check<ExternalException>(stateReply != nullptr, "Failed to query state atom from X server!");
		xcb_atom_t state = stateReply->atom;
		free(stateReply);

		//Request the Motif hints atom from the X server
		xcb_intern_atom_cookie_t motifCookie = xcb_intern_atom(connection, 0, 8, "_MOTIF_WM_HINTS");
		xcb_intern_atom_reply_t* motifReply = xcb_intern_atom_reply(connection, motifCookie, nullptr);
		Check<ExternalException>(motifReply != nullptr, "Failed to query state atom from X server!");
		xcb_atom_t motif = motifReply->atom;
		free(motifReply);

		//Remove fullscreen state if needed
		if(mode != Window::Mode::Fullscreen) {
			xcb_delete_property(connection, window, state);
		}

		//Define Motif hints (for decorations)
		struct MotifHints {
			uint32_t flags = 2;
			uint32_t functions = 0;
			uint32_t decorations = 0;
			int32_t input_mode = 0;
			uint32_t status = 0;
		} hints;

		//Do X stuff
		switch(mode) {
			case Window::Mode::Windowed:
				//Change decorations hints
				hints.flags = 1;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, motif, XCB_ATOM_ATOM, 32, 5, &hints);
				break;
			case Window::Mode::Borderless: {
				//Figure out which output we're on


				//Change decorations hints
				hints.flags = 0;
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, motif, XCB_ATOM_ATOM, 32, 5, &hints);

				break;
			}
			case Window::Mode::Fullscreen: {
				//Request the fullscreen state atom from the X server
				xcb_intern_atom_cookie_t fullscreenCookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_STATE_FULLSCREEN"), "_NET_WM_STATE_FULLSCREEN");
				xcb_intern_atom_reply_t* fullscreenReply = xcb_intern_atom_reply(connection, fullscreenCookie, nullptr);
				Check<ExternalException>(fullscreenReply != nullptr, "Failed to query fullscreen state atom from X server!");
				xcb_atom_t fullscreen = fullscreenReply->atom;
				free(fullscreenReply);

				//Set the fullscreen property
				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, state, XCB_ATOM_ATOM, 32, 1, &fullscreen);
				break;
			}
		}

		//Send any unprocessed commands to the X server and wait
		xcb_flush(connection);
	}
}