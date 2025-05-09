#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "X11Types.hpp"
#include "../LinuxRouter.hpp"

#include <memory>
#include <xcb/xcb_icccm.h>

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
		xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, centered.x, centered.y, win.size.x,
			win.size.y, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0, nullptr);

		//Set initial title
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, win.title.size(), win.title.c_str());

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
		window = 0;
	}

	void X11Common::HandleEvents() {
		xcb_generic_event_t* event;
		while((event = xcb_poll_for_event(connection)) != nullptr) {
			//Handle window close
			if(event->response_type == XCB_CLIENT_MESSAGE || event->response_type == XCB_DESTROY_NOTIFY) {
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

	const glm::uvec2 X11Common::ContentAreaSize() {
		//Request the window geometry from the X server
		xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, window);
		xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(connection, cookie, nullptr);
		Check<ExternalException>(reply != nullptr, "Failed to get window geometry from X server!");
		return {reply->width, reply->height};
	}
}