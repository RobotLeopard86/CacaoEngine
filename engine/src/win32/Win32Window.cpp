#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Input.hpp"
#include "Win32Types.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"

#include <memory>

#include <windowsx.h>
#include <winuser.h>

#include "eternal.hpp"

constexpr DWORD windowedStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
constexpr DWORD fullscreenStyle = WS_POPUP;

namespace Cacao {
	struct Win32WinRegistrar {
		Win32WinRegistrar() {
			Win32WindowImpl::Impl::registry.insert_or_assign("win32", []() { return std::make_unique<Win32WindowImpl>(); });
		}
	};
	__attribute__((used)) Win32WinRegistrar w32wr;

	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch(msg) {
			case WM_DESTROY:
				hWnd = nullptr;
				PostQuitMessage(0);
				return 0;
			case WM_CLOSE:
				Engine::Get().Quit();
				return 0;
			case WM_SIZE: {
				if(wp != SIZE_MAXIMIZED && wp != SIZE_RESTORED) break;
				DataEvent<glm::uvec2> wre("WindowResize", {LOWORD(lp), HIWORD(lp)});
				EventManager::Get().Dispatch(wre);
				return 0;
			}
			case WM_KEYDOWN: {
				DataEvent<unsigned int> kde("KeyDown", IMPL(Window).ConvertKeycode(wp));
				EventManager::Get().Dispatch(kde);
				return 0;
			}
			case WM_KEYUP: {
				DataEvent<unsigned int> kue("KeyUp", IMPL(Window).ConvertKeycode(wp));
				EventManager::Get().Dispatch(kue);
				return 0;
			}
			case WM_LBUTTONDOWN: {
				DataEvent<unsigned int> event("MousePress", CACAO_BUTTON_LEFT);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_LBUTTONUP: {
				DataEvent<unsigned int> event("MouseRelease", CACAO_BUTTON_LEFT);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_MBUTTONDOWN: {
				DataEvent<unsigned int> event("MousePress", CACAO_BUTTON_MIDDLE);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_MBUTTONUP: {
				DataEvent<unsigned int> event("MouseRelease", CACAO_BUTTON_MIDDLE);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_RBUTTONDOWN: {
				DataEvent<unsigned int> event("MousePress", CACAO_BUTTON_RIGHT);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_RBUTTONUP: {
				DataEvent<unsigned int> event("MouseRelease", CACAO_BUTTON_RIGHT);
				EventManager::Get().Dispatch(event);
				return 0;
			}
			case WM_MOUSEMOVE: {
				DataEvent<glm::dvec2> mme("MouseMove", {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)});
				EventManager::Get().Dispatch(mme);
				return 0;
			}
			case WM_MOUSEWHEEL: {
				DataEvent<glm::dvec2> mme("MouseScroll", {0, double(GET_WHEEL_DELTA_WPARAM(wp)) / WHEEL_DELTA});
				if(mme.GetData().y < 1) return 0;
				EventManager::Get().Dispatch(mme);
				return 0;
			}
			case WM_MOUSEHWHEEL: {
				DataEvent<glm::dvec2> mme("MouseScroll", {double(GET_WHEEL_DELTA_WPARAM(wp)) / WHEEL_DELTA, 0});
				if(mme.GetData().x < 1) return 0;
				EventManager::Get().Dispatch(mme);
				return 0;
			}
		}
		return DefWindowProcA(hWnd, msg, wp, lp);
	}

	void Win32WindowImpl::CreateWindow() {
		//Get HINSTANCE
		hInst = GetModuleHandle(nullptr);

		//Register window class
		WNDCLASSEXA winCls = {};
		winCls.cbSize = sizeof(WNDCLASSEXA);
		winCls.hInstance = hInst;
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
		wndclass = RegisterClassExA(&winCls);
		Check<ExternalException>(wndclass != 0, "Failed to register window class!");

		//Create window
		hWnd = CreateWindowExA(WS_EX_APPWINDOW, MAKEINTATOM(wndclass), title.c_str(),
			(mode == Window::Mode::Windowed ? windowedStyle : fullscreenStyle) | (visible ? WS_VISIBLE : 0),
			CW_USEDEFAULT, CW_USEDEFAULT, size.x, size.y, nullptr, nullptr, hInst, nullptr);
		Check<ExternalException>(hWnd != 0, "Failed to create window!");

		//Connect graphics
		PAL::Get().GfxConnect();
	}

	void Win32WindowImpl::DestroyWindow() {
		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Destroy the window
		::DestroyWindow(hWnd);

		//Unregister class
		UnregisterClassA("CacaoEngineWndClass", hInst);
	}

	void Win32WindowImpl::HandleEvents() {
		MSG msg = {};
		while(PeekMessageA(&msg, hWnd, 0, 0, TRUE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	bool Win32WindowImpl::Minimized() {
		return IsIconic(hWnd);
	}

	const glm::uvec2 Win32WindowImpl::ContentAreaSize() {
		RECT rect = {};
		Check<ExternalException>(GetClientRect(hWnd, &rect) != 0, "Failed to retrieve client rectangle!");
		return {rect.right, rect.bottom};
	}

	void Win32WindowImpl::Visibility(bool visible) {
		ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
	}

	void Win32WindowImpl::Title(const std::string& newTitle) {
		SetWindowTextA(hWnd, newTitle.c_str());
	}

	void Win32WindowImpl::Resize(const glm::uvec2& newSize) {
		SetWindowPos(hWnd, nullptr, 0, 0, newSize.x, newSize.y, SWP_NOMOVE | SWP_NOZORDER);
	}

	void Win32WindowImpl::SaveWinPos() {
		RECT r = {};
		Check<ExternalException>(GetWindowRect(hWnd, &r) != 0, "Failed to retrieve window rectangle!");
		lastPos = {r.left, r.top};
	}

	void Win32WindowImpl::SaveWinSize() {
		RECT r = {};
		Check<ExternalException>(GetWindowRect(hWnd, &r) != 0, "Failed to retrieve window rectangle!");
		lastSize = {r.right - r.left, r.bottom - r.top};
	}

	void Win32WindowImpl::RestoreWin() {
		SetWindowPos(hWnd, nullptr, lastPos.x, lastPos.y, lastSize.x, lastSize.y, SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	void Win32WindowImpl::ModeChange(Window::Mode newMode) {
		//Get monitor info
		MONITORINFOEXA mi = {};
		mi.cbSize = sizeof(MONITORINFOEXA);
		HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfoA(hMon, &mi);

		//Do the mode switch
		switch(newMode) {
			case Window::Mode::Windowed:
				//Restore desktop mode
				if(mode == Window::Mode::Fullscreen) ChangeDisplaySettingsExA(mi.szDevice, nullptr, nullptr, CDS_FULLSCREEN, nullptr);

				//Set style
				SetWindowLongPtrA(hWnd, GWL_STYLE, windowedStyle | WS_VISIBLE);
				SetWindowLongPtrA(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
				break;

			case Window::Mode::Borderless:
				//Put window in foreground
				SetForegroundWindow(hWnd);

				//Set style
				SetWindowLongPtrA(hWnd, GWL_STYLE, fullscreenStyle | WS_VISIBLE);
				SetWindowLongPtrA(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

				//Set position and size
				SetWindowPos(hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
				size = {mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top};
				break;

			case Window::Mode::Fullscreen:
				//Configure monitor
				DEVMODEA dm = {};
				dm.dmSize = sizeof(DEVMODEA);
				EnumDisplaySettingsExA(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm, 0);
				dm.dmPelsWidth = size.x;
				dm.dmPelsHeight = size.y;
				dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
				Check<ExternalException>(ChangeDisplaySettingsExA(mi.szDevice, &dm, nullptr, CDS_FULLSCREEN, nullptr) == DISP_CHANGE_SUCCESSFUL, "Failed to change to fullscreen display mode! Changes have not been applied.");

				//Put window in foreground
				SetForegroundWindow(hWnd);

				//Set style
				SetWindowLongPtrA(hWnd, GWL_STYLE, fullscreenStyle | WS_VISIBLE);
				SetWindowLongPtrA(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

				//Set position and size
				SetWindowPos(hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
		}
	}

	unsigned int Win32WindowImpl::ConvertKeycode(unsigned int key) {
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({
			{VK_RETURN, CACAO_KEY_ENTER},
			{VK_ESCAPE, CACAO_KEY_ESCAPE},
			{VK_BACK, CACAO_KEY_BACKSPACE},
			{VK_TAB, CACAO_KEY_TAB},
			{VK_SPACE, CACAO_KEY_SPACE},
			{VK_OEM_7, CACAO_KEY_APOSTROPHE},
			{VK_OEM_COMMA, CACAO_KEY_COMMA},
			{VK_OEM_MINUS, CACAO_KEY_MINUS},
			{VK_OEM_PLUS, CACAO_KEY_EQUALS},
			{VK_OEM_PERIOD, CACAO_KEY_PERIOD},
			{VK_OEM_2, CACAO_KEY_SLASH},
			{0x30, CACAO_KEY_0},
			{0x31, CACAO_KEY_1},
			{0x32, CACAO_KEY_2},
			{0x33, CACAO_KEY_3},
			{0x34, CACAO_KEY_4},
			{0x35, CACAO_KEY_5},
			{0x36, CACAO_KEY_6},
			{0x37, CACAO_KEY_7},
			{0x38, CACAO_KEY_8},
			{0x39, CACAO_KEY_9},
			{VK_OEM_1, CACAO_KEY_SEMICOLON},
			{VK_OEM_4, CACAO_KEY_LEFT_BRACKET},
			{VK_OEM_6, CACAO_KEY_RIGHT_BRACKET},
			{VK_OEM_5, CACAO_KEY_BACKSLASH},
			{VK_OEM_3, CACAO_KEY_GRAVE_ACCENT},
			{0x41, CACAO_KEY_A},
			{0x42, CACAO_KEY_B},
			{0x43, CACAO_KEY_C},
			{0x44, CACAO_KEY_D},
			{0x45, CACAO_KEY_E},
			{0x46, CACAO_KEY_F},
			{0x47, CACAO_KEY_G},
			{0x48, CACAO_KEY_H},
			{0x49, CACAO_KEY_I},
			{0x4A, CACAO_KEY_J},
			{0x4B, CACAO_KEY_K},
			{0x4C, CACAO_KEY_L},
			{0x4D, CACAO_KEY_M},
			{0x4E, CACAO_KEY_N},
			{0x4F, CACAO_KEY_O},
			{0x50, CACAO_KEY_P},
			{0x51, CACAO_KEY_Q},
			{0x52, CACAO_KEY_R},
			{0x53, CACAO_KEY_S},
			{0x54, CACAO_KEY_T},
			{0x55, CACAO_KEY_U},
			{0x56, CACAO_KEY_V},
			{0x57, CACAO_KEY_W},
			{0x58, CACAO_KEY_X},
			{0x59, CACAO_KEY_Y},
			{0x5A, CACAO_KEY_Z},
			{VK_CAPITAL, CACAO_KEY_CAPS_LOCK},
			{VK_F1, CACAO_KEY_F1},
			{VK_F2, CACAO_KEY_F2},
			{VK_F3, CACAO_KEY_F3},
			{VK_F4, CACAO_KEY_F4},
			{VK_F5, CACAO_KEY_F5},
			{VK_F6, CACAO_KEY_F6},
			{VK_F7, CACAO_KEY_F7},
			{VK_F8, CACAO_KEY_F8},
			{VK_F9, CACAO_KEY_F9},
			{VK_F10, CACAO_KEY_F10},
			{VK_F11, CACAO_KEY_F11},
			{VK_F12, CACAO_KEY_F12},
			{VK_SNAPSHOT, CACAO_KEY_PRINT_SCREEN},
			{VK_SCROLL, CACAO_KEY_SCROLL_LOCK},
			{VK_PAUSE, CACAO_KEY_PAUSE},
			{VK_INSERT, CACAO_KEY_INSERT},
			{VK_DELETE, CACAO_KEY_DELETE},
			{VK_HOME, CACAO_KEY_HOME},
			{VK_PRIOR, CACAO_KEY_PAGE_UP},
			{VK_END, CACAO_KEY_END},
			{VK_NEXT, CACAO_KEY_PAGE_DOWN},
			{VK_RIGHT, CACAO_KEY_RIGHT},
			{VK_LEFT, CACAO_KEY_LEFT},
			{VK_DOWN, CACAO_KEY_DOWN},
			{VK_UP, CACAO_KEY_UP},
			{VK_NUMLOCK, CACAO_KEY_NUM_LOCK},
			{VK_DIVIDE, CACAO_KEY_KP_DIVIDE},
			{VK_MULTIPLY, CACAO_KEY_KP_MULTIPLY},
			{VK_SUBTRACT, CACAO_KEY_KP_MINUS},
			{VK_ADD, CACAO_KEY_KP_PLUS},
			{VK_NUMPAD1, CACAO_KEY_KP_1},
			{VK_NUMPAD2, CACAO_KEY_KP_2},
			{VK_NUMPAD3, CACAO_KEY_KP_3},
			{VK_NUMPAD4, CACAO_KEY_KP_4},
			{VK_NUMPAD5, CACAO_KEY_KP_5},
			{VK_NUMPAD6, CACAO_KEY_KP_6},
			{VK_NUMPAD7, CACAO_KEY_KP_7},
			{VK_NUMPAD8, CACAO_KEY_KP_8},
			{VK_NUMPAD9, CACAO_KEY_KP_9},
			{VK_NUMPAD0, CACAO_KEY_KP_0},
			{VK_DECIMAL, CACAO_KEY_KP_PERIOD},
			{VK_LCONTROL, CACAO_KEY_LEFT_CONTROL},
			{VK_LSHIFT, CACAO_KEY_LEFT_SHIFT},
			{VK_LMENU, CACAO_KEY_LEFT_ALT},
			{VK_LWIN, CACAO_KEY_LEFT_SUPER},
			{VK_RCONTROL, CACAO_KEY_RIGHT_CONTROL},
			{VK_RSHIFT, CACAO_KEY_RIGHT_SHIFT},
			{VK_RMENU, CACAO_KEY_RIGHT_ALT},
			{VK_RWIN, CACAO_KEY_RIGHT_SUPER},
		});
		if(codes.contains(key)) return codes.at(key);
		return UINT32_MAX;
	}

	unsigned int Win32WindowImpl::ConvertButtonCode(unsigned int button) {
		//Windows doesn't use this
		return button;
	}
}