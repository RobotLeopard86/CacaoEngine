#include "../Context.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "ImplAccessor.hpp"
#include "Win32Types.hpp"

#include "glad/gl.h"
#include "glad/wgl.h"

namespace Cacao {
	struct Context::Impl {
		HDC dc;
		HGLRC ctx;
	};

	Context::Context() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//=============== SET UP FAKE WINDOW TO LOAD WGL ===============
		{
			//Make a fake window
			HWND dummyWnd = CreateWindowExA(WS_EX_NOACTIVATE, MAKEINTATOM(WIN_IMPL(Win32).wndclass), "dummy", 0, CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, nullptr, nullptr, WIN_IMPL(Win32).hInst, nullptr);
			Check<ExternalException>(dummyWnd, "Failed to create dummy WGL window!");

			//Obtain device context (DC)
			HDC dummyDC = GetDC(dummyWnd);
			Check<ExternalException>(dummyDC, "Failed to obtain dummy WGL window device context!", [&dummyWnd]() { DestroyWindow(dummyWnd); });

			//Make a PFD
			PIXELFORMATDESCRIPTOR dummyPFD = {
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				32,
				//Why are these all zero? They don't matter, that's why, but we have to include them
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				24, 8, 0,
				PFD_MAIN_PLANE,
				//Again not important
				0, 0, 0, 0};
			int dummyPF = ChoosePixelFormat(dummyDC, &dummyPFD);
			Check<ExternalException>(dummyPF != 0, "Failed to choose dummy WGL window pixel format!", [&dummyWnd, &dummyDC]() { ReleaseDC(dummyWnd, dummyDC); DestroyWindow(dummyWnd); });
			Check<ExternalException>(SetPixelFormat(dummyDC, dummyPF, &dummyPFD), "Failed to set dummy WGL window pixel format!");

			//Make a temporary context
			HGLRC dummyCtx = wglCreateContext(dummyDC);
			Check<ExternalException>(dummyCtx != nullptr, "Failed to create dummy WGL context!", [&dummyWnd, &dummyDC]() {ReleaseDC(dummyWnd, dummyDC); DestroyWindow(dummyWnd); });
			wglMakeCurrent(dummyDC, dummyCtx);

			//And finally, after all of that, we can access all of WGL
			//Now, Micro$oft, did you have to make it this hard? No. No you didn't.
			gladLoaderLoadWGL(dummyDC);

			//And now we clean up the mess
			//Goodbye, we won't miss you
			wglDeleteContext(dummyCtx);
			ReleaseDC(dummyWnd, dummyDC);
			DestroyWindow(dummyWnd);
		}

		//Obtain REAL device context (DC)
		impl->dc = GetDC(WIN_IMPL(Win32).hWnd);
		Check<ExternalException>(impl->dc, "Failed to obtain window device context!");

		//Get supported extensions list
		std::string extsList(wglGetExtensionsStringARB(impl->dc));
		std::vector<std::string> exts;
		std::stringstream ss(extsList);
		std::string tmp;
		while(std::getline(ss, tmp, ' ')) exts.push_back(tmp);

		//Make sure all extensions are supported
#define CHECK_FOR_EXT(ext) Check<ExternalException>(std::find(exts.cbegin(), exts.cend(), #ext) != exts.cend(), "Required WGL extension " #ext " is not available!", [this]() { ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc); });
		CHECK_FOR_EXT(WGL_ARB_create_context)
		CHECK_FOR_EXT(WGL_ARB_create_context_profile)
		CHECK_FOR_EXT(WGL_ARB_pixel_format)
		CHECK_FOR_EXT(WGL_EXT_swap_control)
#undef CHECK_FOR_EXT
		Check<ExternalException>(
			std::find(exts.cbegin(), exts.cend(), "WGL_ARB_framebuffer_sRGB") != exts.cend() || std::find(exts.cbegin(), exts.cend(), "WGL_EXT_framebuffer_sRGB") != exts.cend(),
			"Required WGL sRGB framebuffer extension is not available!", [this]() { ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc); });

		//Create real pixel format
		int pixelFormat = -1;
		UINT numFormats = 0;
		const int pfAttrs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0};
		Check<ExternalException>(wglChoosePixelFormatARB(impl->dc, pfAttrs, nullptr, 1, &pixelFormat, &numFormats) == TRUE, "Failed to select pixel format!", [this]() { ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc); });
		Check<ExternalException>(numFormats >= 1, "Failed to select pixel format!", [this]() { ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc); });
		Check<ExternalException>(SetPixelFormat(impl->dc, pixelFormat, nullptr), "Failed to set pixel format!");

		//Create real context
		const int ctxAttrs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 1,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0};
		impl->ctx = wglCreateContextAttribsARB(impl->dc, nullptr, ctxAttrs);
		Check<ExternalException>(impl->ctx, "Failed to create OpenGL context!", [this]() { Logger::Engine(Logger::Level::Error) << "0x" << std::hex << GetLastError() << std::dec; ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc); });

		//Make context current
		wglMakeCurrent(impl->dc, impl->ctx);

		//Load OpenGL functions
		Check<ExternalException>(gladLoadGL([](const char* procname) -> GLADapiproc {
			//Try WGL first
			GLADapiproc proc = (GLADapiproc)wglGetProcAddress(procname);
			if(proc) return proc;
			static HMODULE glmod = LoadLibraryA("opengl32.dll");
			return (GLADapiproc)GetProcAddress(glmod, procname);
		}) != 0,
			"Failed to load OpenGL functions!");

		//Enable sRGB rendering
		glEnable(GL_FRAMEBUFFER_SRGB);

		//Yield the context so the GPU manager can have it
		wglMakeCurrent(impl->dc, nullptr);
	}

	Context::~Context() {
		//Destroy context
		wglDeleteContext(impl->ctx);

		//Release device context
		ReleaseDC(WIN_IMPL(Win32).hWnd, impl->dc);
	}

	void Context::SetVSync(bool vsync) {
		wglSwapIntervalEXT(vsync ? 1 : 0);
	}

	void Context::SwapBuffers() {
		::SwapBuffers(impl->dc);
	}

	void Context::MakeCurrent() {
		wglMakeCurrent(impl->dc, impl->ctx);
	}

	void Context::Yield() {
		wglMakeCurrent(impl->dc, nullptr);
	}
}