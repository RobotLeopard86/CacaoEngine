#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventSystem.hpp"
#ifdef HAS_WAYLAND
#include "wayland/WaylandTypes.hpp"
#endif
#ifdef HAS_X11
#include "x11/X11Types.hpp"
#endif
#include "LinuxRouter.hpp"

#include <memory>
#include <cstdlib>

namespace Cacao {
	Window::Impl::Impl() {}
	Window::Impl::~Impl() {}

	Window::Window()
	  : open(false), visible(false), mode(Mode::Windowed), size(0, 0), title(""), lastPos(0, 0) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->useX = Engine::Get().GetInitConfig().forceX11;
#ifndef HAS_X11
		Check<MiscException>(false, "X11 has been disabled in this build, but has been forcibly enabled!");
#endif
#ifdef HAS_WAYLAND
		if(!impl->useX) impl->wl = std::make_unique<WaylandCommon>();
#endif
#ifdef HAS_X11
		impl->x = std::make_unique<X11Common>();
#endif

		//Decide whether to use Wayland or X11 based on environment variables
#define GETENV(v) []() { const char* p = std::getenv(v); if(p == nullptr) return std::string(""); else return std::string(p); }()
		std::string sessionType = GETENV("XDG_SESSION_TYPE");
		std::string xDisplay = GETENV("DISPLAY");
		std::string wlDisplay = GETENV("WAYLAND_DISPLAY");
#undef GETENV
#ifdef HAS_X11
		if(impl->useX && !xDisplay.empty()) {
			goto choice_made;
		}
#endif
		if(!sessionType.empty()) {
			if(sessionType.compare("wayland") == 0) {
#ifdef HAS_WAYLAND
				if(!wlDisplay.empty()) {
					impl->useX = false;
					goto choice_made;
				}
#endif
#ifdef HAS_X11
				if(!xDisplay.empty()) {
					impl->useX = true;
					goto choice_made;
				}
#endif
			}
#ifdef HAS_X11
			if(sessionType.compare("x11") == 0) {
				if(!xDisplay.empty()) {
					impl->useX = true;
					goto choice_made;
				}
			}
#endif
		} else {
#ifdef HAS_WAYLAND
			if(!wlDisplay.empty()) {
				impl->useX = false;
				goto choice_made;
			}
#endif
#ifdef HAS_X11
			if(!xDisplay.empty()) {
				impl->useX = true;
				goto choice_made;
			}
		}
#endif
		Check<MiscException>(false, "Failed to resolve choice of windowing system!");
	choice_made:
		Logger::Engine(Logger::Level::Info) << "Running under " << (impl->useX ? "X11" : "Wayland") << ".";
	}

	Window::~Window() {
		if(open) Close();
	}

#if defined(HAS_X11) && defined(HAS_WAYLAND)
#define FORWARD(fn, ...)          \
	if(impl->useX)                \
		impl->x->fn(__VA_ARGS__); \
	else                          \
		impl->wl->fn(__VA_ARGS__);

#define FORWARD_RET(fn, ...)             \
	if(impl->useX)                       \
		return impl->x->fn(__VA_ARGS__); \
	else                                 \
		return impl->wl->fn(__VA_ARGS__);
#elif defined(HAS_X11)
#define FORWARD(fn, ...) impl->x->fn(__VA_ARGS__);
#define FORWARD_RET(fn, ...) return impl->x->fn(__VA_ARGS__);
#elif defined(HAS_WAYLAND)
#define FORWARD(fn, ...) impl->wl->fn(__VA_ARGS__);
#define FORWARD_RET(fn, ...) return impl->wl->fn(__VA_ARGS__);
#endif

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!open, "The window must not be open when Open is called!");
		Check<BadStateException>(visible || (!visible && mode == Mode::Windowed), "Cannot open the window to a non-windowed mode while invisible!");

		//Set properties
		this->title = title;
		this->size = size;
		this->visible = visible;
		this->mode = mode;

		//Forward call
		FORWARD(CreateWindow)

		open = true;

		//Apply initial mode
		SetMode(mode);
	}

	void Window::Close() {
		Check<BadInitStateException>(open, "The window must be open when Close is called!");

		open = false;

		//Forward call
		FORWARD(DestroyWindow)
	}

	void Window::HandleOSEvents() {
		Check<BadInitStateException>(open, "The window must be open to set the mode!");

		//Forward call
		FORWARD(HandleEvents)
	}

	bool Window::IsMinimized() {
		if(!open) return true;

		//Forward call
		FORWARD_RET(Minimized)
	}

	const glm::uvec2 Window::GetContentAreaSize() {
		if(!open) return {0, 0};

		//Forward call
		FORWARD_RET(ContentAreaSize)
	}

	void Window::Show() {
		Check<BadInitStateException>(open, "The window must be open to show it!");
		Check<BadInitStateException>(!visible, "The window must be hidden when Show is called!");

		visible = true;
		FORWARD(Visibility, true)
	}

	void Window::Hide() {
		Check<BadInitStateException>(open, "The window must be open to hide it!");
		Check<BadInitStateException>(visible, "The window must be shown when Hide is called!");

		visible = false;
		FORWARD(Visibility, false)
	}

	void Window::SetTitle(const std::string& newTitle) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newTitle.length() > 0, "Cannot set window title to an empty string!");

		title = newTitle;
		FORWARD(Title, newTitle)
	}

	void Window::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newSize.x > 0 && newSize.y > 0, "New window size must not have any zero or negative coordinates!");

		size = newSize;
		FORWARD(Resize, newSize)

		//Fire a window resize event
		DataEvent<glm::uvec2> wre("WindowResize", size);
		EventManager::Get().Dispatch(wre);
	}

	void Window::SetMode(Mode newMode) {
		if(mode == newMode) return;
		Check<BadInitStateException>(open, "The window must be open to set the mode!");
		Check<BadStateException>(visible, "The window must be visible to set the mode!");

		//Save last position and size if needed
		if(mode == Mode::Windowed) {
			FORWARD(SaveWinPos)
			FORWARD(SaveWinSize)
		}

		//Forward call
		FORWARD(ModeChange, newMode)

		//Apply last window position and size if applicable
		if(newMode == Mode::Windowed) {
			FORWARD(RestoreWin)
		}

		//Fire a window resize event
		DataEvent<glm::uvec2> wre("WindowResize", size);
		EventManager::Get().Dispatch(wre);

		mode = newMode;
	}
}