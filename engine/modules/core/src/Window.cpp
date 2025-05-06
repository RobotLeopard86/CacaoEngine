#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "PALCommon.hpp"
#include "PALWindow.hpp"
#include "PALImpl.hpp"

namespace Cacao {
	PAL_BACKED_IMPL(Window)

	template<>
	void PAL::ConfigureImplPtr<Window>(Window& w) {
		Check<PALModule, MiscException>(impl->win, "There must be a loaded windowing module to configure a window implementation pointer!");
		Check<NonexistentValueException>(impl->win->factories.contains(PALModule::FactoryType::Window), "Loaded windowing module does not contain a window implementation pointer factory!");

		w.impl->pal = std::dynamic_pointer_cast<PALWindowInterface>(impl->win->factories.at(PALModule::FactoryType::Window)());
	}

	Window::Window()
	  : open(false), visible(false), mode(Mode::Windowed), size(0, 0), title(""), lastPos(0, 0) {
		//Configure implementation pointer
		impl = std::make_unique<Impl>();
		PAL::Get().ConfigureImplPtr<Window>(*this);
	}

	Window::~Window() {
		if(open) Close();
	}

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!open, "The window must not be open when Open is called!");
		Check<BadValueException>(title.length() > 0, "Cannot open a window with zero-length title!");
		Check<BadValueException>(size.x > 0 && size.y > 0, "Cannot open a window with a zero size component!");
		Check<BadValueException>(size.x > 0 && size.y > 0, "Cannot open a window with a zero size component!");

		//Set props
		this->title = title;
		this->size = size;
		this->visible = visible;
		this->mode = mode;

		//Create native window
		impl->pal->CreateNativeWindow();
	}

	void Window::Close() {
		Check<BadInitStateException>(open, "The window must be open when Close is called!");

		//Destroy native window
		impl->pal->DestroyNativeWindow();
	}

	void Window::Show() {
		Check<BadInitStateException>(open, "The window must be open to be shown!");
		Check<BadInitStateException>(!visible, "The window must not be visible when Show is called!");

		visible = true;
		impl->pal->ApplyVisibility();
	}

	void Window::Hide() {
		Check<BadInitStateException>(open, "The window must be open to be hidden!");
		Check<BadInitStateException>(visible, "The window must be visible when Hide is called!");

		visible = false;
		impl->pal->ApplyVisibility();
	}

	void Window::SetTitle(const std::string& newTitle) {
		Check<BadInitStateException>(open, "The window must be open to change the title!");
		Check<BadValueException>(title.length() > 0, "New window title must not be empty!");

		title = newTitle;
		impl->pal->ApplyTitle();
	}

	void Window::SetMode(Mode newMode) {
		Check<BadInitStateException>(open, "The window must be open to change the mode!");
		Check<BadStateException>(visible || (!visible && newMode == Mode::Windowed), "The window must not be invisible to change to a non-Windowed mode!");

		mode = newMode;
		impl->pal->ApplyMode();
	}

	void Window::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(open, "The window must be open to change the size!");
		Check<BadValueException>(size.x > 0 && size.y > 0, "New window size must not have any zero size components!");

		size = newSize;
		impl->pal->ApplySize();
	}

	const glm::uvec2 Window::GetContentAreaSize() {
		if(!open) return {0, 0};
		return impl->pal->GetContentAreaSize();
	}
}