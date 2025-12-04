#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/EventManager.hpp"
#include "ImplAccessor.hpp"
#import "MacOSTypes.hpp"

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <memory>

constexpr NSWindowStyleMask windowedStyle = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable);

namespace Cacao {
	struct MacWinRegistrar {
		MacWinRegistrar() {
			MacOSWindowImpl::Impl::registry.insert_or_assign("cocoa", []() { return std::make_unique<MacOSWindowImpl>(); });
		}
	};
	__attribute__((used)) MacWinRegistrar xwr;

	void MacOSWindowImpl::CreateWindow() {
		@autoreleasepool {
			//Setup app and delegates
			app = [CacaoApp sharedApplication];
			del = [[CacaoAppDelegate alloc] init];
			[app setActivationPolicy:NSApplicationActivationPolicyRegular];
			[app setDelegate:del];

			//Setup menu
			NSMenu* mainMenu = [[NSMenu alloc] init];
			NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
			NSMenu* appMenu = [[NSMenu alloc] init];
			NSString* appName = [[NSProcessInfo processInfo] processName];
			NSMenuItem* aboutMenuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"About %@", appName]
																   action:@selector(orderFrontStandardAboutPanel:)
															keyEquivalent:@""];
			[aboutMenuItem setTarget:app];
			[appMenu addItem:aboutMenuItem];
			[appMenu addItem:[NSMenuItem separatorItem]];
			NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
																  action:@selector(terminate:)
														   keyEquivalent:@"q"];
			[quitMenuItem setTarget:app];
			[appMenu addItem:quitMenuItem];
			[appMenuItem setSubmenu:appMenu];
			[mainMenu addItem:appMenuItem];
			[app setMainMenu:mainMenu];

			//Make frame
			NSRect frame = NSMakeRect(0, 0, size.x, size.y);

			//Create window and delegate
			wdel = [[CacaoWinDelegate alloc] init];
			win = [[NSWindow alloc] initWithContentRect:frame styleMask:windowedStyle backing:NSBackingStoreBuffered defer:NO];
			[win setTitle:[[NSString alloc] initWithCString:title.c_str() encoding:[NSString defaultCStringEncoding]]];
			[win setIsVisible:visible];
			[win setDelegate:wdel];
			[win makeKeyAndOrderFront:nil];

			//Connect graphics
			PAL::Get().GfxConnect();

			//Start app loop
			[app activateIgnoringOtherApps:YES];
			[app run];
		}
	}

	void MacOSWindowImpl::DestroyWindow() {
		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Close window and release delegates
		[win close];
		[wdel release];
		[del release];
		[app release];
	}

	void MacOSWindowImpl::HandleEvents() {
		@autoreleasepool {
			NSEvent* event;
			while((event = [app nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES])) {
				[app sendEvent:event];
				[app updateWindows];
			}
		}
	}

	bool MacOSWindowImpl::Minimized() {
		return win.miniaturized;
	}

	const glm::uvec2 MacOSWindowImpl::ContentAreaSize() {
		return {win.contentView.frame.size.width, win.contentView.frame.size.height};
	}

	void MacOSWindowImpl::Visibility(bool visible) {
		[win setIsVisible:visible];
	}

	void MacOSWindowImpl::Title(const std::string& newTitle) {
		[win setTitle:[[NSString alloc] initWithCString:newTitle.c_str() encoding:[NSString defaultCStringEncoding]]];
	}

	void MacOSWindowImpl::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newSize.x > 0 && newSize.y > 0, "New window size must not have any zero or negative coordinates!");

		size = newSize;
		[win setFrame:NSMakeRect(0, 0, newSize.x, newSize.y) display:YES];

		DataEvent<glm::uvec2> wre("WindowResize", newSize);
		EventManager::Get().Dispatch(wre);
	}

	void MacOSWindowImpl::ModeChange(Window::Mode newMode) {
		@autoreleasepool {
			//Get screen info
			NSScreen* scr = [win screen];

			//Do the mode switch
			switch(newMode) {
				case Window::Mode::Windowed:
					//Leave exclusive fullscreen if needed
					if(mode == Window::Mode::Fullscreen) [win.contentView exitFullScreenModeWithOptions:nil];

					//Restore presentation options if needed
					if(mode == Window::Mode::Borderless) [app setPresentationOptions:lastPresentOpts];

					//Let window be obscured
					[win setLevel:NSNormalWindowLevel];

					//Set style
					[win setStyleMask:windowedStyle];

					//Apply last position and size
					size = lastSize;
					break;

				case Window::Mode::Borderless:
					@autoreleasepool {
						//Put window in foreground and keep it there
						[win orderFrontRegardless];
						[win setLevel:NSFloatingWindowLevel];

						//Hide menubar and dock
						lastPresentOpts = [app presentationOptions];
						[app setPresentationOptions:(NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar)];

						//Set style
						[win setStyleMask:(NSWindowStyleMaskBorderless)];

						//Set window size to screen size
						NSRect screenFrame = [win frameRectForContentRect:scr.frame];
						[win setFrame:screenFrame display:YES animate:YES];
						size = {screenFrame.size.width, screenFrame.size.height};
					}
					break;

				case Window::Mode::Fullscreen:
					@autoreleasepool {
						//Put window in foreground
						[win orderFrontRegardless];

						//Enter fullscreen
						NSDictionary<NSViewFullScreenModeOptionKey, id>* opts = @{
							NSFullScreenModeApplicationPresentationOptions : @(NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar | NSApplicationPresentationDisableProcessSwitching | NSApplicationPresentationFullScreen | NSApplicationPresentationDisableHideApplication)
						};
						Check<ExternalException>([win.contentView enterFullScreenMode:scr withOptions:opts], "Failed to enter fullscreen mode!");
					}
					break;
			}
		}
	}

	void MacOSWindowImpl::SaveWinPos() {
		@autoreleasepool {
			NSRect frame = win.frame;
			lastPos = {frame.origin.x, frame.origin.y};
		}
	}

	void MacOSWindowImpl::SaveWinSize() {
		@autoreleasepool {
			NSRect frame = win.frame;
			lastPos = {frame.size.width, frame.size.height};
		}
	}

	void MacOSWindowImpl::RestoreWin() {
		@autoreleasepool {
			NSRect frame = NSMakeRect(lastPos.x, lastPos.y, lastSize.x, lastSize.y);
			[win setFrame:frame display:YES];
		}
	}

	unsigned int MacOSWindowImpl::ConvertKeycode(unsigned int key) {
		//TODO
		return key;
	}

	unsigned int MacOSWindowImpl::ConvertButtonCode(unsigned int button) {
		//TODO
		return button;
	}
}