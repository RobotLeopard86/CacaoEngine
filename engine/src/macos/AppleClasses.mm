#import "MacOSTypes.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Input.hpp"
#include "ImplAccessor.hpp"

#include <atomic>

#include "glm/glm.hpp"

static std::atomic_bool quitRequested;

@implementation CacaoAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
	@autoreleasepool {
		NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
											location:NSMakePoint(0, 0)
									   modifierFlags:0
										   timestamp:0
										windowNumber:0
											 context:nil
											 subtype:0
											   data1:0
											   data2:0];
		[NSApp postEvent:event atStart:YES];
	}
	[NSApp stop:nil];
}
@end

@implementation CacaoWinDelegate

- (void)windowWillClose:(id)sender {
	if(!quitRequested) [WIN_IMPL(MacOS).app terminate:sender];
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {
	DataEvent<glm::uvec2> wre("WindowResize", {frameSize.width, frameSize.height});
	EventManager::Get().Dispatch(wre);
	return frameSize;
}

@end

@implementation CacaoWin

- (void)mouseDown:(NSEvent*)event {
	DataEvent<unsigned int> e("MousePress", CACAO_BUTTON_LEFT);
	EventManager::Get().Dispatch(e);
}

- (void)mouseUp:(NSEvent*)event {
	DataEvent<unsigned int> e("MouseRelease", CACAO_BUTTON_LEFT);
	EventManager::Get().Dispatch(e);
}

- (void)rightMouseDown:(NSEvent*)event {
	DataEvent<unsigned int> e("MousePress", CACAO_BUTTON_RIGHT);
	EventManager::Get().Dispatch(e);
}

- (void)rightMouseUp:(NSEvent*)event {
	DataEvent<unsigned int> e("MouseRelease", CACAO_BUTTON_RIGHT);
	EventManager::Get().Dispatch(e);
}

//macOS doesn't have a formal way to identify middle click, so we assume that any other button than left or right is middle as there aren't usually that many more buttons on mice
- (void)otherMouseDown:(NSEvent*)event {
	DataEvent<unsigned int> e("MousePress", CACAO_BUTTON_MIDDLE);
	EventManager::Get().Dispatch(e);
}

- (void)otherMouseUp:(NSEvent*)event {
	DataEvent<unsigned int> e("MouseRelease", CACAO_BUTTON_MIDDLE);
	EventManager::Get().Dispatch(e);
}

- (void)mouseMoved:(NSEvent*)event {
	DataEvent<glm::dvec2> mme("MouseMove", {self.mouseLocationOutsideOfEventStream.x, self.mouseLocationOutsideOfEventStream.y});
	EventManager::Get().Dispatch(mme);
}

- (void)scrollWheel:(NSEvent*)event {
	//Get the raw scrolling values
	glm::dvec2 scrollDeltas(event.scrollingDeltaX, event.scrollingDeltaY);

	//If we don't have precise scrolling, we need to do some shenanigans
	if(!event.hasPreciseScrollingDeltas) {
		//Retrieve the system font line height
		CGFloat lineHeight = [self.layoutMgr defaultLineHeightForFont:[NSFont systemFontOfSize:[NSFont systemFontSize]]];

		//Scale the deltas
		scrollDeltas *= lineHeight;
	}

	//Dispatch event
	DataEvent<glm::dvec2> mse("MouseScroll", scrollDeltas);
	EventManager::Get().Dispatch(mse);
}

- (void)keyDown:(NSEvent*)event {
	if(event.ARepeat) return;
	DataEvent<unsigned int> e("KeyDown", IMPL(Window).ConvertKeycode(event.keyCode));
	EventManager::Get().Dispatch(e);
}

- (void)keyUp:(NSEvent*)event {
	DataEvent<unsigned int> e("KeyUp", IMPL(Window).ConvertKeycode(event.keyCode));
	EventManager::Get().Dispatch(e);
}

@end

@implementation CacaoApp

- (instancetype)init {
	self = [super init];
	quitRequested.store(false, std::memory_order_release);
	return self;
}

- (void)terminate:(id)sender {
	quitRequested.store(true, std::memory_order_release);
	Engine::Get().Quit();
}

@end