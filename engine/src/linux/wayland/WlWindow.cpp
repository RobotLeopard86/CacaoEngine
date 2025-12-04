#include "Cacao/Event.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Input.hpp"
#include "WaylandTypes.hpp"

#include "xdg-output-unstable-v1-client-protocol.h"
#include "eternal.hpp"

#include <cstdint>

#include <sys/types.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input-event-codes.h>

namespace Cacao {
	struct WaylandWinRegistrar {
		WaylandWinRegistrar() {
			Window::Impl::registry.insert_or_assign("wayland", []() { return std::make_unique<WaylandWindowImpl>(); });
		}
	};
	__attribute__((used)) WaylandWinRegistrar wwr;

	void WaylandWindowImpl::CreateWindow() {
		//Connect to Wayland
		display = wl_display_connect(nullptr);
		Check<ExternalException>(display != nullptr, "Failed to connect to Wayland display!");

		//Get the registry
		registry = wl_display_get_registry(display);

		//Add registry listener to fetch compositor
		wl_registry_listener listener = {};
		listener.global = [](void* selfp, wl_registry* reg, uint32_t id, const char* iface, uint32_t ver) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);
			std::string interface(iface);
			if(interface.compare(wl_compositor_interface.name) == 0) {
				self->compositor = (wl_compositor*)wl_registry_bind(reg, id, &wl_compositor_interface, ver);
			} else if(interface.compare(zxdg_output_manager_v1_interface.name) == 0) {
				self->outMgr = (zxdg_output_manager_v1*)wl_registry_bind(reg, id, &zxdg_output_manager_v1_interface, ver);
			} else if(interface.compare(wl_seat_interface.name) == 0) {
				self->seat = (wl_seat*)wl_registry_bind(reg, id, &wl_seat_interface, ver);
			}
		};
		listener.global_remove = [](void*, wl_registry*, uint32_t) {};
		wl_registry_add_listener(registry, &listener, this);

		//Fetch global objects
		wl_display_roundtrip(display);
		wl_display_roundtrip(display);
		Check<ExternalException>(compositor != nullptr, "Failed to get Wayland compositor!");
		Check<ExternalException>(outMgr != nullptr, "Failed to get Wayland output manager!");
		Check<ExternalException>(seat != nullptr, "Failed to get Wayland input seat!");

		//Create surface
		surface = wl_compositor_create_surface(compositor);
		Check<ExternalException>(surface != nullptr, "Failed to create surface!");

		//Let Wayland process surface creation
		wl_display_roundtrip(display);

		//Obtain input devices
		keyboard = wl_seat_get_keyboard(seat);
		Check<ExternalException>(keyboard != nullptr, "Failed to obtain keyboard!");
		mouse = wl_seat_get_pointer(seat);
		Check<ExternalException>(keyboard != nullptr, "Failed to obtain mouse!");

		//Create keyboard listener
		keyboardListener = {};
		keyboardListener.keymap = [](void* selfp, wl_keyboard*, uint32_t, int fd, uint32_t size) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);

			//Get keymap string
			//We have to use memory mapping; Wayland seems to love that for some reason...
			char* keymapStr = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
			Check<ExternalException>(keymapStr != MAP_FAILED, "Failed to get keymap string!", [fd]() { close(fd); });

			//Setup XKB context
			self->xkb = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
			Check<ExternalException>(self->xkb, "Failed to create XKB context!", [fd, keymapStr, size]() { munmap(keymapStr, size); close(fd); });

			//Create XKB keymap
			self->keymap = xkb_keymap_new_from_string(self->xkb, keymapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

			//Clean up memory mapping stuff
			munmap(keymapStr, size);
			close(fd);

			//Check for keymap
			//We don't do this immediately because no matter what we want to clean up the keymap string, so no need for an unwind function and this makes it easier
			Check<ExternalException>(self->keymap, "Failed to create XKB keymap!");

			//Create XKB state
			self->xkbState = xkb_state_new(self->keymap);
			Check<ExternalException>(self->xkbState, "Failed to create XKB state!");
		};
		keyboardListener.key = [](void* selfp, wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);

			//Get keysym
			//You may be wondering why we add 8 to the keysym. Apparently this is a Wayland thing. I don't know why and frankly I don't want to know why.
			xkb_keysym_t keysym = xkb_state_key_get_one_sym(self->xkbState, key + 8);

			//Dispatch event
			DataEvent<unsigned int> event((state == WL_KEYBOARD_KEY_STATE_RELEASED ? "KeyUp" : "KeyDown"), self->ConvertKeycode(keysym));
			EventManager::Get().Dispatch(event);
		};
		keyboardListener.enter = [](void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*) {};
		keyboardListener.leave = [](void*, wl_keyboard*, uint32_t, wl_surface*) {};
		keyboardListener.modifiers = [](void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {};
		keyboardListener.repeat_info = [](void*, wl_keyboard*, int32_t, int32_t) {};
		wl_keyboard_add_listener(keyboard, &keyboardListener, this);

		//Create mouse (pointer) listener
		mouseListener = {};
		mouseListener.motion = [](void*, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y) {
			//Convert coordinates
			glm::dvec2 mousePos(wl_fixed_to_double(x), wl_fixed_to_double(y));

			//Dispatch event
			DataEvent<glm::dvec2> event("MouseMove", mousePos);
			EventManager::Get().Dispatch(event);
		};
		mouseListener.axis = [](void* selfp, wl_pointer*, uint32_t, uint32_t axis, wl_fixed_t value) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);
			double val = wl_fixed_to_double(value);
			if(axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
				self->scrollAccumulator.y += val;
			} else if(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
				self->scrollAccumulator.x += val;
			}
		};
		mouseListener.frame = [](void* selfp, wl_pointer*) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);

			//Dispatch accumulated scroll event
			DataEvent<glm::dvec2> event("MouseScroll", self->scrollAccumulator);
			EventManager::Get().Dispatch(event);

			//Reset accumulator
			self->scrollAccumulator = {0, 0};
		};
		mouseListener.button = [](void* selfp, wl_pointer*, uint32_t, uint32_t, uint32_t button, uint32_t state) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);

			//Get converted button code
			unsigned int buttonCode = self->ConvertButtonCode(button);

			//Dispatch event
			DataEvent<unsigned int> event((state == WL_POINTER_BUTTON_STATE_PRESSED ? "MousePress" : "MouseRelease"), buttonCode);
			EventManager::Get().Dispatch(event);
		};
		mouseListener.enter = [](void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t) {};
		mouseListener.leave = [](void*, wl_pointer*, uint32_t, wl_surface*) {};
		mouseListener.axis_source = [](void*, wl_pointer*, uint32_t) {};
		mouseListener.axis_stop = [](void*, wl_pointer*, uint32_t, uint32_t) {};
		mouseListener.axis_discrete = [](void*, wl_pointer*, uint32_t, int32_t) {};
		mouseListener.axis_value120 = [](void*, wl_pointer*, uint32_t, int32_t) {};
		mouseListener.axis_relative_direction = [](void*, wl_pointer*, uint32_t, uint32_t) {};
		wl_pointer_add_listener(mouse, &mouseListener, this);

		//Create output listener
		outListener = {};
		outListener.logical_size = [](void* selfp, zxdg_output_v1*, int lw, int lh) {
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);
			self->outputSize = {lw, lh};
		};
		outListener.logical_position = [](void*, zxdg_output_v1*, int, int) {};
		outListener.done = [](void*, zxdg_output_v1*) {};
		outListener.name = [](void*, zxdg_output_v1*, const char*) {};
		outListener.description = [](void*, zxdg_output_v1*, const char*) {};

		//Create and register surface listener
		surfListener = {};
		surfListener.enter = [](void* selfp, wl_surface*, wl_output* output) {
			//Get xdg_output
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);
			self->out = zxdg_output_manager_v1_get_xdg_output(self->outMgr, output);

			//Register size listener
			zxdg_output_v1_add_listener(self->out, &(self->outListener), self);
		};
		surfListener.leave = [](void* selfp, wl_surface*, wl_output*) {
			//Release xdg_output object
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(selfp);
			zxdg_output_v1_destroy(self->out);
		};
		surfListener.preferred_buffer_scale = [](void*, wl_surface*, int) {};
		surfListener.preferred_buffer_transform = [](void*, wl_surface*, unsigned int) {};
		wl_surface_add_listener(surface, &surfListener, this);

		//Set a fake content size so the graphics system can initialize (this might not be accurate, but the resize event will correct it)
		lastKnownContentSize = size;

		//Connect the graphics system (the shenanigans with open are for the GetContentAreaSize method to work)
		open = true;
		PAL::Get().GfxConnect();
		open = false;

		//Initialize libdecor interfaces
		decorInterface.error = [](libdecor*, libdecor_error, const char* msg) {
			std::stringstream emsg;
			emsg << "Libdecor encountered an error: " << msg;
			Logger::Engine(Logger::Level::Error) << emsg.str();
			Check<ExternalException>(false, emsg.str());
		};
		frameInterface.close = [](libdecor_frame*, void*) {
			Engine::Get().Quit();
		};
		frameInterface.commit = [](libdecor_frame*, void*) {};
		frameInterface.configure = [](libdecor_frame* frame, libdecor_configuration* cfg, void* usr) {
			//Get this
			WaylandWindowImpl* self = reinterpret_cast<WaylandWindowImpl*>(usr);

			//Get new size
			glm::ivec2 newSize = {0, 0};
			libdecor_configuration_get_content_size(cfg, frame, &newSize.x, &newSize.y);

			//Update window size
			if(newSize.x <= 0 || newSize.y <= 0) {
				newSize.x = self->size.x;
				newSize.y = self->size.y;
			} else {
				self->size = newSize;
			}

			//Fire a window resize event
			DataEvent<glm::uvec2> wre("WindowResize", self->size);
			EventManager::Get().Dispatch(wre);

			//Set new libdecor state
			libdecor_state* state = libdecor_state_new(self->size.x, self->size.y);
			libdecor_frame_commit(frame, state, cfg);
			libdecor_state_free(state);

			//Mark us as configured
			self->configured = true;

			//Save size
			self->lastKnownContentSize = newSize;
		};
	}

	void WaylandWindowImpl::DestroyWindow() {
		//Destroy libdecor objects
		Visibility(false);

		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Destroy XKB objects
		xkb_state_unref(xkbState);
		xkb_keymap_unref(keymap);
		xkb_context_unref(xkb);

		//Destroy input objects
		wl_pointer_destroy(mouse);
		wl_keyboard_destroy(keyboard);
		wl_seat_destroy(seat);

		//Destroy output manager
		zxdg_output_manager_v1_destroy(outMgr);

		//Destroy surface
		wl_surface_destroy(surface);

		//Destroy registry
		wl_registry_destroy(registry);

		//Disconnect from Wayland display
		wl_display_disconnect(display);
	}

	void WaylandWindowImpl::HandleEvents() {
		//Dispatch libdecor events
		auto ldd = libdecor_dispatch(decor, 0);
		//Check<ExternalException>(ldd >= 0, "Failed to dispatch libdecor event!");
		if(ldd < 0) {
			std::stringstream emsg;
			if(errno == EPROTO) {
				uint32_t protoErr = 0, id = 0;
				const wl_interface* interface;
				protoErr = wl_display_get_protocol_error(display, &interface, &id);
				if(protoErr != 0) {
					emsg << "Protocol error #" << protoErr << " on object " << id;
					if(interface) {
						emsg << " (" << interface->name << ")";
					}
					emsg << ".";
				} else {
					emsg << "Non-protocol error.";
				}
			} else {
				emsg << "Unknown error.";
			}
			Check<ExternalException>(false, emsg.str());
		}

		//Dispatch pending Wayland events
		int ret = wl_display_dispatch_pending(display);
		wl_display_flush(display);
		if(ret == -1) {
			int err = wl_display_get_error(display);
			std::stringstream emsg;
			emsg << "Wayland event processing error #" << err << "; ";
			if(err == EPROTO) {
				uint32_t protoErr = 0, id = 0;
				const wl_interface* interface;
				protoErr = wl_display_get_protocol_error(display, &interface, &id);
				if(protoErr != 0) {
					emsg << "Protocol error #" << protoErr << " on object " << id;
					if(interface) {
						emsg << " (" << interface->name << ")";
					}
					emsg << ".";
				} else {
					emsg << "Non-protocol error.";
				}
			} else {
				emsg << "Unknown error.";
			}
			Check<ExternalException>(false, emsg.str());
		}
	}

	const glm::uvec2 WaylandWindowImpl::ContentAreaSize() {
		return lastKnownContentSize;
	}

	void WaylandWindowImpl::Visibility(bool visible) {
		if(visible) {
			//Setup libdecor
			decor = libdecor_new(display, &decorInterface);
			Check<ExternalException>(decor != nullptr, "Failed to create libdecor context!");
			frame = libdecor_decorate(decor, surface, &frameInterface, this);
			Check<ExternalException>(frame != nullptr, "Failed to create libdecor frame!");

			//Set attributes (the app ID will be changed to that of the primary bundle when that is implemented)
			libdecor_frame_set_app_id(frame, Engine::Get().GetInitConfig().clientID.id.c_str());
			libdecor_frame_set_title(frame, title.c_str());

			//Map frame and start configuration
			libdecor_frame_map(frame);

			//Let Wayland process decoration setup
			wl_display_roundtrip(display);
			wl_display_roundtrip(display);

			//Wait until libdecor configuration is done
			while(!configured) {
				Check<ExternalException>(libdecor_dispatch(decor, 0) >= 0, "Failed to dispatch one or more libdecor events while waiting for configure!");
			}
		} else {
			//Destroy libdecor objects
			libdecor_frame_unref(frame);
			libdecor_unref(decor);

			//Attach a null buffer to hide the surface
			wl_surface_attach(surface, nullptr, 0, 0);
			wl_surface_commit(surface);
		}
	}

	void WaylandWindowImpl::Title(const std::string& title) {
		libdecor_frame_set_title(frame, title.c_str());
	}

	void WaylandWindowImpl::Resize(const glm::uvec2& size) {
		//Set new libdecor state
		libdecor_state* state = libdecor_state_new(size.x, size.y);
		libdecor_frame_commit(frame, state, nullptr);
		libdecor_state_free(state);
	}

	void WaylandWindowImpl::ModeChange(Window::Mode mode) {
		//Unset fullscreen if leaving exclusive mode
		if(mode != Window::Mode::Fullscreen) {
			libdecor_frame_unset_fullscreen(frame);
		}

		//Do a roundtrip to avoid sync issues
		wl_display_roundtrip(display);

		switch(mode) {
			case Window::Mode::Windowed:
				//Enable decorations
				libdecor_frame_set_visibility(frame, true);
				size = lastSize;
				break;
			case Window::Mode::Borderless:
				//Disable decorations
				libdecor_frame_set_visibility(frame, false);

				//Set new libdecor state
				{
					libdecor_state* state = libdecor_state_new(outputSize.x, outputSize.y);
					libdecor_frame_commit(frame, state, nullptr);
					libdecor_state_free(state);
				}
				size = outputSize;
				break;
			case Window::Mode::Fullscreen:
				//Disable decorations
				libdecor_frame_set_visibility(frame, false);

				//Become fullscreen
				libdecor_frame_set_fullscreen(frame, nullptr);
				break;
		}
	}

	void WaylandWindowImpl::SaveWinSize() {
		lastSize = lastKnownContentSize;
	}

	void WaylandWindowImpl::RestoreWin() {
		//Set new libdecor state
		libdecor_state* state = libdecor_state_new(lastSize.x, lastSize.y);
		libdecor_frame_commit(frame, state, nullptr);
		libdecor_state_free(state);
	}

	unsigned int WaylandWindowImpl::ConvertKeycode(unsigned int key) {
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({{CACAO_KEY_ENTER, XKB_KEY_Return},
			{CACAO_KEY_ESCAPE, XKB_KEY_Escape},
			{CACAO_KEY_BACKSPACE, XKB_KEY_BackSpace},
			{CACAO_KEY_TAB, XKB_KEY_Tab},
			{CACAO_KEY_SPACE, XKB_KEY_space},
			{CACAO_KEY_EXCLAMATION, XKB_KEY_exclam},
			{CACAO_KEY_APOSTROPHE, XKB_KEY_apostrophe},
			{CACAO_KEY_COMMA, XKB_KEY_comma},
			{CACAO_KEY_MINUS, XKB_KEY_minus},
			{CACAO_KEY_EQUALS, XKB_KEY_equal},
			{CACAO_KEY_PERIOD, XKB_KEY_period},
			{CACAO_KEY_SLASH, XKB_KEY_slash},
			{CACAO_KEY_0, XKB_KEY_0},
			{CACAO_KEY_1, XKB_KEY_1},
			{CACAO_KEY_2, XKB_KEY_2},
			{CACAO_KEY_3, XKB_KEY_3},
			{CACAO_KEY_4, XKB_KEY_4},
			{CACAO_KEY_5, XKB_KEY_5},
			{CACAO_KEY_6, XKB_KEY_6},
			{CACAO_KEY_7, XKB_KEY_7},
			{CACAO_KEY_8, XKB_KEY_8},
			{CACAO_KEY_9, XKB_KEY_9},
			{CACAO_KEY_SEMICOLON, XKB_KEY_semicolon},
			{CACAO_KEY_LEFT_BRACKET, XKB_KEY_bracketleft},
			{CACAO_KEY_RIGHT_BRACKET, XKB_KEY_bracketright},
			{CACAO_KEY_BACKSLASH, XKB_KEY_backslash},
			{CACAO_KEY_GRAVE_ACCENT, XKB_KEY_grave},
			{CACAO_KEY_A, XKB_KEY_a},
			{CACAO_KEY_B, XKB_KEY_b},
			{CACAO_KEY_C, XKB_KEY_c},
			{CACAO_KEY_D, XKB_KEY_d},
			{CACAO_KEY_E, XKB_KEY_e},
			{CACAO_KEY_F, XKB_KEY_f},
			{CACAO_KEY_G, XKB_KEY_g},
			{CACAO_KEY_H, XKB_KEY_h},
			{CACAO_KEY_I, XKB_KEY_i},
			{CACAO_KEY_J, XKB_KEY_j},
			{CACAO_KEY_K, XKB_KEY_k},
			{CACAO_KEY_L, XKB_KEY_l},
			{CACAO_KEY_M, XKB_KEY_m},
			{CACAO_KEY_N, XKB_KEY_n},
			{CACAO_KEY_O, XKB_KEY_o},
			{CACAO_KEY_P, XKB_KEY_p},
			{CACAO_KEY_Q, XKB_KEY_q},
			{CACAO_KEY_R, XKB_KEY_r},
			{CACAO_KEY_S, XKB_KEY_s},
			{CACAO_KEY_T, XKB_KEY_t},
			{CACAO_KEY_U, XKB_KEY_u},
			{CACAO_KEY_V, XKB_KEY_v},
			{CACAO_KEY_W, XKB_KEY_w},
			{CACAO_KEY_X, XKB_KEY_x},
			{CACAO_KEY_Y, XKB_KEY_y},
			{CACAO_KEY_Z, XKB_KEY_z},
			{CACAO_KEY_CAPS_LOCK, XKB_KEY_Caps_Lock},
			{CACAO_KEY_F1, XKB_KEY_F1},
			{CACAO_KEY_F2, XKB_KEY_F2},
			{CACAO_KEY_F3, XKB_KEY_F3},
			{CACAO_KEY_F4, XKB_KEY_F4},
			{CACAO_KEY_F5, XKB_KEY_F5},
			{CACAO_KEY_F6, XKB_KEY_F6},
			{CACAO_KEY_F7, XKB_KEY_F7},
			{CACAO_KEY_F8, XKB_KEY_F8},
			{CACAO_KEY_F9, XKB_KEY_F9},
			{CACAO_KEY_F10, XKB_KEY_F10},
			{CACAO_KEY_F11, XKB_KEY_F11},
			{CACAO_KEY_F12, XKB_KEY_F12},
			{CACAO_KEY_PRINT_SCREEN, XKB_KEY_Print},
			{CACAO_KEY_SCROLL_LOCK, XKB_KEY_Scroll_Lock},
			{CACAO_KEY_PAUSE, XKB_KEY_Pause},
			{CACAO_KEY_INSERT, XKB_KEY_Insert},
			{CACAO_KEY_DELETE, XKB_KEY_Delete},
			{CACAO_KEY_HOME, XKB_KEY_Home},
			{CACAO_KEY_PAGE_UP, XKB_KEY_Page_Up},
			{CACAO_KEY_END, XKB_KEY_End},
			{CACAO_KEY_PAGE_DOWN, XKB_KEY_Page_Down},
			{CACAO_KEY_RIGHT, XKB_KEY_Right},
			{CACAO_KEY_LEFT, XKB_KEY_Left},
			{CACAO_KEY_DOWN, XKB_KEY_Down},
			{CACAO_KEY_UP, XKB_KEY_Up},
			{CACAO_KEY_NUM_LOCK, XKB_KEY_Num_Lock},
			{CACAO_KEY_KP_DIVIDE, XKB_KEY_KP_Divide},
			{CACAO_KEY_KP_MULTIPLY, XKB_KEY_KP_Multiply},
			{CACAO_KEY_KP_MINUS, XKB_KEY_KP_Subtract},
			{CACAO_KEY_KP_PLUS, XKB_KEY_KP_Add},
			{CACAO_KEY_KP_ENTER, XKB_KEY_KP_Enter},
			{CACAO_KEY_KP_1, XKB_KEY_KP_1},
			{CACAO_KEY_KP_2, XKB_KEY_KP_2},
			{CACAO_KEY_KP_3, XKB_KEY_KP_3},
			{CACAO_KEY_KP_4, XKB_KEY_KP_4},
			{CACAO_KEY_KP_5, XKB_KEY_KP_5},
			{CACAO_KEY_KP_6, XKB_KEY_KP_6},
			{CACAO_KEY_KP_7, XKB_KEY_KP_7},
			{CACAO_KEY_KP_8, XKB_KEY_KP_8},
			{CACAO_KEY_KP_9, XKB_KEY_KP_9},
			{CACAO_KEY_KP_0, XKB_KEY_KP_0},
			{CACAO_KEY_KP_PERIOD, XKB_KEY_KP_Decimal},
			{CACAO_KEY_LEFT_CONTROL, XKB_KEY_Control_L},
			{CACAO_KEY_LEFT_SHIFT, XKB_KEY_Shift_L},
			{CACAO_KEY_LEFT_ALT, XKB_KEY_Alt_L},
			{CACAO_KEY_LEFT_SUPER, XKB_KEY_Super_L},
			{CACAO_KEY_RIGHT_CONTROL, XKB_KEY_Control_R},
			{CACAO_KEY_RIGHT_SHIFT, XKB_KEY_Shift_R},
			{CACAO_KEY_RIGHT_ALT, XKB_KEY_Alt_R},
			{CACAO_KEY_RIGHT_SUPER, XKB_KEY_Super_R}});
		if(codes.contains(key)) return codes.at(key);
		return UINT32_MAX;
	}

	unsigned int WaylandWindowImpl::ConvertButtonCode(unsigned int button) {
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({{BTN_LEFT, CACAO_BUTTON_LEFT},
			{BTN_MIDDLE, CACAO_BUTTON_MIDDLE},
			{BTN_RIGHT, CACAO_BUTTON_RIGHT}});
		if(codes.contains(button)) return codes.at(button);
		return UINT32_MAX;
	}
}