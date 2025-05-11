#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "WaylandTypes.hpp"

#include <memory>
#include "xdg-shell-client-protocol.h"

#define win Window::Get()

namespace Cacao {
	void WaylandCommon::CreateWindow() {
		//Connect to Wayland
		display = wl_display_connect(nullptr);
		Check<ExternalException>(display != nullptr, "Failed to connect to Wayland display!");

		//Get the registry
		reg = wl_display_get_registry(display);

		//Add registry listener to fetch compositor
		wl_registry_listener listener = {};
		listener.global = [](void* selfp, wl_registry* r, uint32_t id, const char* iface, uint32_t ver) {
			WaylandCommon* self = reinterpret_cast<WaylandCommon*>(selfp);
			std::string interface(iface);
			if(interface.compare("wl_compositor") == 0) {
				self->compositor = (wl_compositor*)wl_registry_bind(r, id, &wl_compositor_interface, ver);
			}
		};
		listener.global_remove = [](void*, wl_registry*, uint32_t) {};
		wl_registry_add_listener(reg, &listener, this);

		//Fetch the compositor
		wl_display_roundtrip(display);
		Check<ExternalException>(compositor != nullptr, "Failed to get Wayland compositor!");

		//Create surface
		surf = wl_compositor_create_surface(compositor);
		Check<ExternalException>(surf != nullptr, "Failed to create surface!");

		//Let Wayland process surface creation
		wl_display_roundtrip(display);

		//Connect the graphics system
		PAL::Get().GfxConnect();

		//Initialize libdecor
		libdecor_interface decorInterface = {};
		decorInterface.error = [](libdecor* ctx, libdecor_error err, const char* msg) {
			std::stringstream emsg;
			emsg << "Libdecor encountered an error: " << msg;
			Check<ExternalException>(false, emsg.str());
		};
		decor = libdecor_new(display, &decorInterface);
		Check<ExternalException>(decor != nullptr, "Failed to create libdecor context!");

		//Decorate window (I wouldn't need to do this if GNOME would be reasonable and add SSD, but...)
		libdecor_frame_interface frameInterface = {};
		frameInterface.close = [](libdecor_frame* frame, void* usr) {
			Engine::Get().Quit();
		};
		bool hasConfigured = false;
		frameInterface.configure = [](libdecor_frame* frame, libdecor_configuration* cfg, void* usr) {
			//Get new size
			glm::ivec2 newSize = {0, 0};
			libdecor_configuration_get_content_size(cfg, frame, &newSize.x, &newSize.y);

			//Update window size
			if(newSize.x < 0 || newSize.y < 0) {
				newSize.x = win.size.x;
				newSize.y = win.size.y;
			} else {
				win.size = newSize;
			}

			//Fire a window resize event
			DataEvent<glm::uvec2> wre("WindowResize", win.size);
			EventManager::Get().Dispatch(wre);

			//Set new libdecor state
			libdecor_state* state = libdecor_state_new(win.size.x, win.size.y);
			libdecor_frame_commit(frame, state, cfg);
			libdecor_state_free(state);

			//Mark us as configured
			*(reinterpret_cast<bool*>(usr)) = true;
		};
		frame = libdecor_decorate(decor, surf, &frameInterface, &hasConfigured);
		Check<ExternalException>(frame != nullptr, "Failed to create libdecor frame!");

		//Set attributes (the app ID will be changed to that of the primary bundle when that is implemented)
		libdecor_frame_set_app_id(frame, "net.rl86.CacaoEngine");
		libdecor_frame_set_title(frame, "Cacao Engine");

		//Map frame and start configuration
		libdecor_frame_map(frame);

		//Let Wayland process decoration setup
		wl_display_roundtrip(display);
		wl_display_roundtrip(display);

		//Wait until libdecor configuration is done
		while(!hasConfigured) {
			Check<ExternalException>(libdecor_dispatch(decor, 0) >= 0, "Failed to dispatch one or more libdecor events while waiting for configure!");
		}
	}

	void WaylandCommon::DestroyWindow() {
		//Destroy libdecor objects
		libdecor_frame_unref(frame);
		libdecor_unref(decor);

		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Destroy surface
		wl_surface_destroy(surf);

		//Destroy registry
		wl_registry_destroy(reg);

		//Disconnect from Wayland display
		wl_display_disconnect(display);
	}

	void WaylandCommon::HandleEvents() {
		//Dispatch libdecor events
		Check<ExternalException>(libdecor_dispatch(decor, 0) >= 0, "Failed to dispatch libdecor event!");

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

	const glm::uvec2 WaylandCommon::ContentAreaSize() {
		return {0, 0};
	}

	void WaylandCommon::Visibility(bool visible) {
	}

	void WaylandCommon::Title(const std::string& title) {
	}

	void WaylandCommon::Resize(const glm::uvec2& size) {
	}

	void WaylandCommon::ModeChange(Window::Mode mode) {
	}

	void WaylandCommon::SaveWinSize() {
	}

	void WaylandCommon::RestoreWin() {
	}
}