#include "Cacao/Event.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "WaylandTypes.hpp"

#include "xdg-output-unstable-v1-client-protocol.h"

#include <cstdint>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

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
		//TODO
		return key;
	}
}