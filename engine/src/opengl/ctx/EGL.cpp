#include "../Context.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "KHR/khrplatform.h"
#include <EGL/eglplatform.h>
#include <wayland-client-core.h>
#include <wayland-egl-core.h>

#ifdef HAS_X11
#include "x11/X11Types.hpp"
#endif

#ifdef HAS_WAYLAND
#include "wayland/WaylandTypes.hpp"

#include <wayland-egl.h>
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sstream>

#include "eternal.hpp"
#include "glad/gl.h"

constexpr const inline auto eglErrors = mapbox::eternal::map<EGLenum, mapbox::eternal::string>({{EGL_NOT_INITIALIZED, "Not Initialized"},
	{EGL_BAD_ACCESS, "Bad Access"},
	{EGL_BAD_ALLOC, "Bad Allocation"},
	{EGL_BAD_ATTRIBUTE, "Bad Attribute"},
	{EGL_BAD_CONTEXT, "Bad Contex"},
	{EGL_BAD_CONFIG, "Bad Config"},
	{EGL_BAD_CURRENT_SURFACE, "Bad Current Surface"},
	{EGL_BAD_DISPLAY, "Bad Display"},
	{EGL_BAD_SURFACE, "Bad Surface"},
	{EGL_BAD_MATCH, "Bad Match"},
	{EGL_BAD_PARAMETER, "Bad Parameter"},
	{EGL_BAD_NATIVE_PIXMAP, "Bad Native Pixmap"},
	{EGL_BAD_NATIVE_WINDOW, "Bad Native Window"},
	{EGL_CONTEXT_LOST, "Context Lost"}});

#define EGL_CHECK(msg)                                                                                                 \
	if(EGLenum err = eglGetError(); err != EGL_SUCCESS) {                                                              \
		std::stringstream message;                                                                                     \
		message << "EGL error 0x" << std::hex << err << std::dec << " (" << eglErrors.at(err).c_str() << "), " << msg; \
		Check<ExternalException>(false, message.str());                                                                \
	}

namespace Cacao {
	struct Context::Impl {
		//EGL objects
		EGLDisplay dpy;
		EGLContext ctx;
		EGLSurface surface;
		EGLConfig cfg;

		//Wayland support
		bool wl;
#ifdef HAS_WAYLAND
		wl_egl_window* wlEglWindow;
		EventConsumer wlEglResize;
#endif
	};

	Context::Context() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Obtain the EGL display
		impl->wl = IMPL(Window).ProviderID().compare("wayland") == 0;
		if(impl->wl) {
#ifdef HAS_WAYLAND
			impl->dpy = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, WIN_IMPL(Wayland).display, nullptr);
#endif
		} else {
#ifdef HAS_X11
			impl->dpy = eglGetPlatformDisplay(EGL_PLATFORM_XCB_EXT, WIN_IMPL(X11).connection, nullptr);
#endif
		}
		Check<ExternalException>(impl->dpy != EGL_NO_DISPLAY, "Failed to get EGL display!");

		//Initialize EGL
		EGLint major, minor;
		Check<ExternalException>(eglInitialize(impl->dpy, &major, &minor) == EGL_TRUE, "Failed to initialize EGL!");
		EGL_CHECK("Failed to initialize EGL!")

		//Configure EGL
		const EGLint eglCfg[] = {
			EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_CONFORMANT, EGL_OPENGL_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			EGL_NONE};
		EGLint numConfigs;
		EGLBoolean success = eglChooseConfig(impl->dpy, eglCfg, &impl->cfg, 1, &numConfigs);
		Check<ExternalException>(success && numConfigs >= 1, "Failed to select EGL config!");
		EGL_CHECK("Failed to select EGL config!")

		//Activate OpenGL API
		Check<ExternalException>(eglBindAPI(EGL_OPENGL_API), "Failed to set graphics API mode!");
		EGL_CHECK("Failed to set graphics API mode!")

		//Create context
		const EGLint contextAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION, 4,
			EGL_CONTEXT_MINOR_VERSION, 1,
			EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
			EGL_NONE};
		impl->ctx = eglCreateContext(impl->dpy, impl->cfg, EGL_NO_CONTEXT, contextAttribs);
		Check<ExternalException>(impl->ctx != EGL_NO_CONTEXT, "Failed to create OpenGL context!");
		EGL_CHECK("Failed to create OpenGL context!")

#ifdef HAS_WAYLAND
		if(impl->wl) {
			//Create Wayland EGL window
			WaylandWindowImpl& wwi = WIN_IMPL(Wayland);
			impl->wlEglWindow = wl_egl_window_create(wwi.surface, wwi.size.x, wwi.size.y);
			Check<ExternalException>(impl->wlEglWindow, "Failed to create Wayland EGL window!");
			wl_display_roundtrip(wwi.display);

			//Register EGL window resizer
			impl->wlEglResize = EventConsumer([this](Event& e) {
				DataEvent<glm::uvec2>& wre = static_cast<DataEvent<glm::uvec2>&>(e);
				wl_egl_window_resize(impl->wlEglWindow, wre.GetData().x, wre.GetData().y, 0, 0);
			});
			EventManager::Get().SubscribeConsumer("WindowResize", impl->wlEglResize);
		}
#endif

		//Create surface
		if(major > 1 && (major >= 1 && minor == 5)) {
			//EGL 1.5+ has this built-in
			void* nativeWin;
			if(impl->wl) {
#ifdef HAS_WAYLAND
				nativeWin = impl->wlEglWindow;
#endif
			} else {
#ifdef HAS_X11
				nativeWin = &WIN_IMPL(X11).window;
#endif
			}
			impl->surface = eglCreatePlatformWindowSurface(impl->dpy, impl->cfg, nativeWin, nullptr);
		} else {
			//Try querying for the extension
			PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC ext =
				(PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");


			if(ext) {
				//Same procedure as with 1.5+
				void* nativeWin;
				if(impl->wl) {
#ifdef HAS_WAYLAND
					nativeWin = impl->wlEglWindow;
#endif
				} else {
#ifdef HAS_X11
					nativeWin = &WIN_IMPL(X11).window;
#endif
				}
				impl->surface = ext(impl->dpy, impl->cfg, nativeWin, nullptr);
			} else {
				//We have to use the old method
				EGLNativeWindowType nativeWin;
				if(impl->wl) {
#ifdef HAS_WAYLAND
					nativeWin = (EGLNativeWindowType)impl->wlEglWindow;
#endif
				} else {
#ifdef HAS_X11
					nativeWin = (khronos_uintptr_t)WIN_IMPL(X11).window;
#endif
				}
				impl->surface = eglCreateWindowSurface(impl->dpy, impl->cfg, nativeWin, nullptr);
			}
		}
		Check<ExternalException>(impl->ctx != EGL_NO_CONTEXT, "Failed to create EGL surface!");
		EGL_CHECK("Failed to create EGL surface!")

		//Make context current
		eglMakeCurrent(impl->dpy, impl->surface, impl->surface, impl->ctx);

		//Load OpenGL functions
		Check<ExternalException>(gladLoadGL(eglGetProcAddress), "Failed to load OpenGL functions!");

		//Enable sRGB rendering
		glEnable(GL_FRAMEBUFFER_SRGB);

		//Yield the context so the GPU manager can have it
		eglMakeCurrent(impl->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	}

	Context::~Context() {
		//Destroy context & surface
		eglDestroyContext(impl->dpy, impl->ctx);
		eglDestroySurface(impl->dpy, impl->surface);

#ifdef HAS_WAYLAND
		if(impl->wl) {
			//Unegister EGL window resizer
			EventManager::Get().UnsubscribeConsumer("WindowResize", impl->wlEglResize);

			//Destroy Wayland EGL window
			wl_egl_window_destroy(impl->wlEglWindow);
		}
#endif

		//Terminate EGL
		eglTerminate(impl->dpy);
	}

	void Context::SetVSync(bool vsync) {
		eglSwapInterval(impl->dpy, vsync ? 1 : 0);
	}

	void Context::SwapBuffers() {
		eglSwapBuffers(impl->dpy, impl->surface);
	}

	void Context::MakeCurrent() {
		eglMakeCurrent(impl->dpy, impl->surface, impl->surface, impl->ctx);
	}

	void Context::Yield() {
		eglMakeCurrent(impl->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	}
}