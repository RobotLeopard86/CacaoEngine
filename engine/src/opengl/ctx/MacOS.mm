#include "ImplAccessor.hpp"
#import "MacOSTypes.hpp"
#include <AppKit/AppKit.h>
#include "Cacao/Exceptions.hpp"
#include <memory>
#include "../Context.hpp"

#define GL_SILENCE_DEPRECATION
#import <OpenGL/OpenGL.h>

#include "glad/gl.h"

static std::string err;

@interface CacaoGLView : NSView
@property (nonatomic, strong) NSOpenGLPixelFormat* pixFormat;
@property (nonatomic, strong) NSOpenGLContext* ctx;

+ (GLADapiproc)lookupNSGLSymbol:(const char*)procname;
@end

@implementation CacaoGLView
static CFBundleRef glFramework;

+ (GLADapiproc)lookupNSGLSymbol:(const char*)procname {
	CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);
	GLADapiproc symbol = (GLADapiproc)CFBundleGetFunctionPointerForName(glFramework, symbolName);
	CFRelease(symbolName);
	return symbol;
}

- (instancetype)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if(!self) return nil;

	//No layer needed
	self.wantsLayer = NO;

	//Load OpenGL framework
	glFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
	CFRetain(glFramework);

	//Create OpenGL context
	const NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion4_1Core,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAClosestPolicy,
		NSOpenGLPFADepthSize, 24,
		0};
	self.pixFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	if(self.pixFormat == nil) {
		err = "Failed to choose pixel format!";
		return nil;
	}
	self.ctx = [[NSOpenGLContext alloc] initWithFormat:self.pixFormat shareContext:nil];
	if(self.ctx == nil) {
		err = "Failed to create OpenGL context!";
		return nil;
	}
	self.ctx.view = self;

	//Make context current
	[self.ctx makeCurrentContext];

	//Load remaining OpenGL functions
	int gladResult = gladLoadGL([](const char* pn) { return [CacaoGLView lookupNSGLSymbol:pn]; });
	if(gladResult == 0) {
		err = "Failed to load OpenGL functions!";
		return nil;
	}

	//Enable sRGB rendering
	glEnable(GL_FRAMEBUFFER_SRGB);

	return self;
}

- (void)dealloc {
	//Unload OpenGL framework
	CFRelease(glFramework);

	[super dealloc];
}

@end

namespace Cacao {
	struct Context::Impl {
		CacaoGLView* view;
	};

	Context::Context() {
		impl = std::make_unique<Impl>();

		//Create OpenGL view (which handles context creation stuff) and make it the content view
		err = "";
		impl->view = [[CacaoGLView alloc] initWithFrame:IMPL(Window).mac->win.frame];
		Check<ExternalException>(impl->view != nil, err);
		[IMPL(Window).mac->win setContentView:impl->view];
	}

	Context::~Context() {
		//Free OpenGL view
		[IMPL(Window).mac->win setContentView:nil];
		[impl->view release];
	}

	void Context::SetVSync(bool vsync) {
		int swap = vsync ? 1 : 0;
		[impl->view.ctx setValues:&swap forParameter:NSOpenGLContextParameterSwapInterval];
	}

	void Context::SwapBuffers() {
		[impl->view.ctx flushBuffer];
	}
}