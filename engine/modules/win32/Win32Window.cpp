#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Win32Types.hpp"

#include <memory>

constexpr DWORD windowedStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
constexpr DWORD fullscreenStyle = WS_POPUP;

namespace Cacao {
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch(msg) {
			case WM_DESTROY:
				hWnd = nullptr;
				PostQuitMessage(0);
				return 0;
			case WM_CLOSE:
				Engine::Get().Quit();
				return 0;
		}
		return DefWindowProcA(hWnd, msg, wp, lp);
	}

	Window::Window()
	  : open(false), visible(false), mode(Mode::Windowed), size(0, 0), title(""), lastPos(0, 0) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->win = std::make_unique<WindowsCommon>();

		//Get hInstance
		impl->win->hInst = GetModuleHandle(nullptr);
	}

	Window::~Window() {
		if(open) Close();
	}

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!open, "The window must not be open when Open is called!");
		Check<BadStateException>(visible || (!visible && mode == Mode::Windowed), "Cannot open the window to a non-windowed mode while invisible!");

		//Set properties
		this->title = title;
		this->size = size;
		this->visible = visible;
		this->mode = mode;

		//Register window class
		WNDCLASSEXA winCls = {};
		winCls.cbSize = sizeof(WNDCLASSEXA);
		winCls.hInstance = impl->win->hInst;
		winCls.lpszClassName = "CacaoEngineWndClass";
		winCls.lpfnWndProc = HandleMsg;
		winCls.cbClsExtra = 0;
		winCls.cbWndExtra = 0;
		winCls.hIcon = nullptr;
		winCls.hIconSm = nullptr;
		winCls.hCursor = nullptr;
		winCls.hbrBackground = nullptr;
		winCls.lpszMenuName = nullptr;
		winCls.style = CS_OWNDC;
		ATOM regResult = RegisterClassExA(&winCls);
		Check<ExternalException>(regResult != 0, "Failed to register window class!");

		//Create window
		impl->win->hWnd = CreateWindowExA(WS_EX_APPWINDOW, MAKEINTATOM(regResult), title.c_str(),
			(mode == Mode::Windowed ? windowedStyle : fullscreenStyle) | (visible ? WS_VISIBLE : 0),
			CW_USEDEFAULT, CW_USEDEFAULT, size.x, size.y, nullptr, nullptr, impl->win->hInst, nullptr);
		Check<ExternalException>(impl->win->hWnd != 0, "Failed to create window!");

		open = true;

		//Apply initial mode
		SetMode(mode);
	}

	void Window::Close() {
		Check<BadInitStateException>(open, "The window must be open when Close is called!");

		open = false;

		//Destroy the window
		DestroyWindow(impl->win->hWnd);

		//Unregister class
		UnregisterClassA("CacaoEngineWndClass", impl->win->hInst);
	}

	void Window::HandleOSEvents() {
		Check<BadInitStateException>(open, "The window must be open to set the mode!");

		MSG msg = {};
		while(PeekMessageA(&msg, impl->win->hWnd, 0, 0, TRUE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	bool Window::IsMinimized() {
		return (open ? IsIconic(impl->win->hWnd) : true);
	}

	const glm::uvec2 Window::GetContentAreaSize() {
		if(!open) return {0, 0};

		RECT rect = {};
		Check<ExternalException>(GetClientRect(impl->win->hWnd, &rect) != 0, "Failed to retrieve client rectangle!");
		return {rect.right, rect.bottom};
	}

	void Window::Show() {
		Check<BadInitStateException>(open, "The window must be open to show it!");
		Check<BadInitStateException>(!visible, "The window must be hidden when Show is called!");

		visible = true;
		ShowWindow(impl->win->hWnd, SW_SHOW);
	}

	void Window::Hide() {
		Check<BadInitStateException>(open, "The window must be open to hide it!");
		Check<BadInitStateException>(visible, "The window must be shown when Hide is called!");

		visible = false;
		ShowWindow(impl->win->hWnd, SW_HIDE);
	}

	void Window::SetTitle(const std::string& newTitle) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newTitle.length() > 0, "Cannot set window title to an empty string!");

		title = newTitle;
		SetWindowTextA(impl->win->hWnd, newTitle.c_str());
	}

	void Window::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newSize.x > 0 && newSize.y > 0, "New window size must not have any zero or negative coordinates!");

		size = newSize;
		SetWindowPos(impl->win->hWnd, nullptr, 0, 0, newSize.x, newSize.y, SWP_NOMOVE | SWP_NOZORDER);
	}

	void Window::SetMode(Mode newMode) {
		if(mode == newMode) return;
		Check<BadInitStateException>(open, "The window must be open to set the mode!");
		Check<BadStateException>(visible, "The window must be visible to set the mode!");

		//Save last position and size if needed
		if(mode == Mode::Windowed) {
			RECT r = {};
			GetWindowRect(impl->win->hWnd, &r);
			lastPos = {r.left, r.top};
			if(newMode == Mode::Fullscreen) {
				lastSize = {r.right - r.left, r.bottom - r.top};
			}
		}

		//Get monitor info
		MONITORINFOEXA mi = {};
		mi.cbSize = sizeof(MONITORINFOEXA);
		HMONITOR hMon = MonitorFromWindow(impl->win->hWnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfoA(hMon, &mi);

		//Do the mode switch
		switch(newMode) {
			case Mode::Windowed:
				//Restore desktop mode
				if(mode == Mode::Fullscreen) ChangeDisplaySettingsExA(mi.szDevice, nullptr, nullptr, CDS_FULLSCREEN, nullptr);

				//Set style
				SetWindowLongPtrA(impl->win->hWnd, GWL_STYLE, windowedStyle | WS_VISIBLE);
				SetWindowLongPtrA(impl->win->hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

				//Apply last position and size
				SetWindowPos(impl->win->hWnd, nullptr, lastPos.x, lastPos.y, lastSize.x, lastSize.y, SWP_NOZORDER | SWP_FRAMECHANGED);
				size = lastSize;
				break;

			case Mode::Borderless:
				//Put window in foreground
				SetForegroundWindow(impl->win->hWnd);

				//Set style
				SetWindowLongPtrA(impl->win->hWnd, GWL_STYLE, fullscreenStyle | WS_VISIBLE);
				SetWindowLongPtrA(impl->win->hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

				//Set position and size
				SetWindowPos(impl->win->hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
				break;

			case Mode::Fullscreen:
				//Configure monitor
				DEVMODEA dm = {};
				dm.dmSize = sizeof(DEVMODEA);
				EnumDisplaySettingsExA(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm, 0);
				dm.dmPelsWidth = size.x;
				dm.dmPelsHeight = size.y;
				dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
				Check<ExternalException>(ChangeDisplaySettingsExA(mi.szDevice, &dm, nullptr, CDS_FULLSCREEN, nullptr) == DISP_CHANGE_SUCCESSFUL, "Failed to change to fullscreen display mode! Changes have not been applied.");

				//Put window in foreground
				SetForegroundWindow(impl->win->hWnd);

				//Set style
				SetWindowLongPtrA(impl->win->hWnd, GWL_STYLE, fullscreenStyle | WS_VISIBLE);
				SetWindowLongPtrA(impl->win->hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

				//Set position and size
				SetWindowPos(impl->win->hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
		}

		mode = newMode;
	}
}