#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Engine.hpp"
#include "WindowImplBase.hpp"

#include <memory>
#include <cstdlib>

namespace Cacao {
	Window::Window() {
		//Create implementation pointer

		//Try preferred provider
		std::string provider = Engine::Get().GetInitConfig().preferredWindowProvider;
		if(Impl::registry.contains(provider)) {
			impl = Impl::registry[provider]();
		}

		//Platform behavior
#ifdef _WIN32
		provider = "win32";
#endif
#ifdef __APPLE__
		provider = "cocoa";
#endif
#ifdef __linux__
		//Decide whether to use Wayland or X11 based on environment variables
#define GETENV(v) []() { const char* p = std::getenv(v); if(p == nullptr) return std::string(""); else return std::string(p); }()
		std::string sessionType = GETENV("XDG_SESSION_TYPE");
		std::string xDisplay = GETENV("DISPLAY");
		std::string wlDisplay = GETENV("WAYLAND_DISPLAY");
#undef GETENV
		bool useX = false;
		if(!sessionType.empty()) {
			if(sessionType.compare("wayland") == 0) {
#ifdef HAS_WAYLAND
				if(!wlDisplay.empty()) {
					useX = false;
					goto choice_made;
				}
#endif
#ifdef HAS_X11
				if(!xDisplay.empty()) {
					useX = true;
					goto choice_made;
				}
#endif
			}
#ifdef HAS_X11
			if(sessionType.compare("x11") == 0) {
				if(!xDisplay.empty()) {
					useX = true;
					goto choice_made;
				}
			}
#endif
		} else {
#ifdef HAS_WAYLAND
			if(!wlDisplay.empty()) {
				useX = false;
				goto choice_made;
			}
#endif
#ifdef HAS_X11
			if(!xDisplay.empty()) {
				useX = true;
				goto choice_made;
			}
		}
#endif
		Check<MiscException>(false, "Failed to resolve choice of Linux windowing system!");
	choice_made:
		Logger::Engine(Logger::Level::Info) << "Running under " << (useX ? "X11" : "Wayland") << ".";
		provider = (useX ? "x11" : "wayland");
#endif

		//Create pointer
		Check<MiscException>(Impl::registry.contains(provider), "No windowing system provider is registered that meets criteria!");
		impl = Impl::registry[provider]();

		//Set initial impl state
		impl->title = "";
		impl->size = {0, 0};
		impl->lastSize = {0, 0};
		impl->lastPos = {0, 0};
		impl->visible = false;
		impl->open = false;
		impl->mode = Mode::Windowed;
	}

	Window::~Window() {
		if(impl->open) Close();
	}

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!impl->open, "The window must not be open when Open is called!");
		Check<BadStateException>(visible || (!visible && mode == Mode::Windowed), "Cannot open the window to a non-windowed mode while invisible!");

		//Set properties
		impl->title = title;
		impl->size = size;
		impl->visible = visible;
		impl->mode = mode;

		//Forward call
		impl->CreateWindow();

		impl->open = true;

		//Apply initial mode
		SetMode(mode);
	}

	void Window::Close() {
		Check<BadInitStateException>(impl->open, "The window must be open when Close is called!");

		impl->open = false;

		//Forward call
		impl->DestroyWindow();
	}

	void Window::HandleOSEvents() {
		Check<BadInitStateException>(impl->open, "The window must be open to set the mode!");

		//Forward call
		impl->HandleEvents();
	}

	bool Window::IsOpen() const {
		return impl->open;
	}

	bool Window::IsVisible() const {
		return impl->open ? impl->visible : false;
	}

	bool Window::IsMinimized() const {
		if(!impl->open) return true;

		//Forward call
		return impl->Minimized();
	}

	const std::string Window::GetTitle() const {
		return impl->open ? impl->title : "";
	}

	const glm::uvec2 Window::GetSize() const {
		return impl->open ? impl->size : glm::uvec2 {0, 0};
	}

	const glm::uvec2 Window::GetContentAreaSize() const {
		if(!impl->open) return {0, 0};

		//Forward call
		return impl->ContentAreaSize();
	}

	Window::Mode Window::GetMode() const {
		return impl->open ? impl->mode : Mode::Windowed;
	}

	void Window::Show() {
		Check<BadInitStateException>(impl->open, "The window must be open to show it!");
		Check<BadInitStateException>(!impl->visible, "The window must be hidden when Show is called!");

		impl->visible = true;
		impl->Visibility(true);
	}

	void Window::Hide() {
		Check<BadInitStateException>(impl->open, "The window must be open to hide it!");
		Check<BadInitStateException>(impl->visible, "The window must be shown when Hide is called!");

		impl->visible = false;
		impl->Visibility(false);
	}

	void Window::SetTitle(const std::string& newTitle) {
		Check<BadInitStateException>(impl->open, "The window must be open to set the title!");
		Check<BadValueException>(newTitle.length() > 0, "Cannot set window title to an empty string!");

		impl->title = newTitle;
		impl->Title(newTitle);
	}

	void Window::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(impl->open, "The window must be open to set the title!");
		Check<BadValueException>(newSize.x > 0 && newSize.y > 0, "New window size must not have any zero or negative coordinates!");

		impl->size = newSize;
		impl->Resize(newSize);

		//Fire a window resize event
		DataEvent<glm::uvec2> wre("WindowResize", impl->size);
		EventManager::Get().Dispatch(wre);
	}

	void Window::SetMode(Mode newMode) {
		if(impl->mode == newMode) return;
		Check<BadInitStateException>(impl->open, "The window must be open to set the mode!");
		Check<BadStateException>(impl->visible, "The window must be visible to set the mode!");

		//Save last position and size if needed
		if(impl->mode == Mode::Windowed) {
			impl->SaveWinPos();
			impl->SaveWinSize();
		}

		//Forward call
		impl->ModeChange(newMode);

		//Apply last window position and size if applicable
		if(newMode == Mode::Windowed) {
			impl->RestoreWin();
		}

		//Fire a window resize event
		DataEvent<glm::uvec2> wre("WindowResize", impl->size);
		EventManager::Get().Dispatch(wre);

		impl->mode = newMode;
	}
}