#import "MacOSTypes.hpp"
#include <atomic>
#include "Cacao/Engine.hpp"
#include "ImplAccessor.hpp"

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
	if(!quitRequested) [IMPL(Window).mac->app terminate:sender];
}

@end

@implementation CacaoApp

- (instancetype)init {
	self = [super init];
	quitRequested.store(false);
	return self;
}

- (void)terminate:(id)sender {
	quitRequested.store(true);
	Engine::Get().Quit();
}

@end