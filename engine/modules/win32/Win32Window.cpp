#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#import "Win32Types.hpp"

#include <memory>

namespace Cacao {
	Window::Window()
	  : open(false), visible(false), mode(Mode::Windowed), size(0, 0), title(""), lastPos(0, 0) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->mac = std::make_unique<WindowsCommon>();
	}

	Window::~Window() {
		if(open) Close();
	}

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!open, "The window must not be open when Open is called!");

		//Set properties
		this->title = title;
		this->size = size;
		this->visible = visible;
		this->mode = mode;

		open = true;
	}

	void Window::Close() {
		Check<BadInitStateException>(open, "The window must be open when Close is called!");

		open = false;
	}
}