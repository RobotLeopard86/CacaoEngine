#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Win32Types.hpp"
#include "Cacao/PAL.hpp"

#include <memory>

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
				DataEvent<glm::uvec2> wre("Window", {LOWORD(lp), HIWORD(lp)});
				EventManager::Get().Dispatch(wre);
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
		ATOM regResult = RegisterClassExA(&winCls);
		Check<ExternalException>(regResult != 0, "Failed to register window class!");

		//Create window
		hWnd = CreateWindowExA(WS_EX_APPWINDOW, MAKEINTATOM(regResult), title.c_str(),
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
}