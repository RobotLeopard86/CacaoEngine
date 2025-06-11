#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/EventSystem.hpp"
#import "MacOSTypes.hpp"
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

#include <memory>

constexpr NSWindowStyleMask windowedStyle = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable);

namespace Cacao {
	Window::Window()
	  : open(false), visible(false), mode(Mode::Windowed), size(0, 0), title(""), lastPos(0, 0) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->mac = std::make_unique<MacOSCommon>();

		//Initialize and configure NSApplication
		@autoreleasepool {
			[CacaoApp sharedApplication];
			impl->mac->del = [[CacaoAppDelegate alloc] init];
			[impl->mac->app setActivationPolicy:NSApplicationActivationPolicyRegular];
			[impl->mac->app setDelegate:impl->mac->del];

			//Setup menu
			NSMenu* mainMenu = [[NSMenu alloc] init];
			NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
			NSMenu* appMenu = [[NSMenu alloc] init];
			NSString* appName = [[NSProcessInfo processInfo] processName];
			NSMenuItem* aboutMenuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"About %@", appName]
																   action:@selector(orderFrontStandardAboutPanel:)
															keyEquivalent:@""];
			[aboutMenuItem setTarget:impl->mac->app];
			[appMenu addItem:aboutMenuItem];
			[appMenu addItem:[NSMenuItem separatorItem]];
			NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
																  action:@selector(terminate:)
														   keyEquivalent:@"q"];
			[quitMenuItem setTarget:impl->mac->app];
			[appMenu addItem:quitMenuItem];
			[appMenuItem setSubmenu:appMenu];
			[mainMenu addItem:appMenuItem];
			[impl->mac->app setMainMenu:mainMenu];
		}
	}

	Window::~Window() {
		if(open) Close();
		[impl->mac->del release];
		[impl->mac->app release];
	}

	void Window::Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode) {
		Check<BadInitStateException>(!open, "The window must not be open when Open is called!");

		//Set properties
		this->title = title;
		this->size = size;
		this->visible = visible;
		this->mode = mode;

		@autoreleasepool {
			//Make frame
			NSRect frame = NSMakeRect(0, 0, size.x, size.y);

			//Create window and delegate
			impl->mac->wdel = [[CacaoWinDelegate alloc] init];
			impl->mac->win = [[NSWindow alloc] initWithContentRect:frame styleMask:windowedStyle backing:NSBackingStoreBuffered defer:NO];
			[impl->mac->win setTitle:[[NSString alloc] initWithCString:title.c_str() encoding:[NSString defaultCStringEncoding]]];
			[impl->mac->win setIsVisible:visible];
			[impl->mac->win setDelegate:impl->mac->wdel];
			[impl->mac->win makeKeyAndOrderFront:nil];

			//Connect graphics
			PAL::Get().GfxConnect();

			//Start app loop
			[impl->mac->app activateIgnoringOtherApps:YES];
			[impl->mac->app run];
		}

		open = true;

		//Apply initial mode
		SetMode(mode);
	}

	void Window::Close() {
		Check<BadInitStateException>(open, "The window must be open when Close is called!");

		open = false;

		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Close window and release delegate
		[impl->mac->win close];
		[impl->mac->wdel release];
	}

	void Window::HandleOSEvents() {
		@autoreleasepool {
			NSEvent* event;
			while((event = [impl->mac->app nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES])) {
				[impl->mac->app sendEvent:event];
				[impl->mac->app updateWindows];
			}
		}
	}

	bool Window::IsMinimized() {
		return (open ? impl->mac->win.miniaturized : true);
	}

	const glm::uvec2 Window::GetContentAreaSize() {
		if(!open) return {0, 0};

		return {impl->mac->win.contentView.frame.size.width, impl->mac->win.contentView.frame.size.height};
	}

	void Window::Show() {
		Check<BadInitStateException>(open, "The window must be open to show it!");
		Check<BadInitStateException>(!visible, "The window must be hidden when Show is called!");

		visible = true;
		[impl->mac->win setIsVisible:YES];
	}

	void Window::Hide() {
		Check<BadInitStateException>(open, "The window must be open to hide it!");
		Check<BadInitStateException>(visible, "The window must be shown when Hide is called!");

		visible = false;
		[impl->mac->win setIsVisible:NO];
	}

	void Window::SetTitle(const std::string& newTitle) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newTitle.length() > 0, "Cannot set window title to an empty string!");

		title = newTitle;
		[impl->mac->win setTitle:[[NSString alloc] initWithCString:newTitle.c_str() encoding:[NSString defaultCStringEncoding]]];
	}

	void Window::Resize(const glm::uvec2& newSize) {
		Check<BadInitStateException>(open, "The window must be open to set the title!");
		Check<BadValueException>(newSize.x > 0 && newSize.y > 0, "New window size must not have any zero or negative coordinates!");

		size = newSize;
		[impl->mac->win setFrame:NSMakeRect(0, 0, newSize.x, newSize.y) display:YES];

		DataEvent<glm::uvec2> wre("Window", newSize);
		EventManager::Get().Dispatch(wre);
	}

	void Window::SetMode(Mode newMode) {
		if(mode == newMode) return;
		Check<BadInitStateException>(open, "The window must be open to set the mode!");
		Check<BadStateException>(visible, "The window must be visible to set the mode!");

		//Save last position and size if needed
		if(mode == Mode::Windowed) {
			NSRect frame = impl->mac->win.frame;
			lastPos = {frame.origin.x, frame.origin.y};
			lastSize = {frame.size.width, frame.size.height};
		}

		@autoreleasepool {
			//Get screen info
			NSScreen* scr = [impl->mac->win screen];

			//Do the mode switch
			switch(newMode) {
				case Mode::Windowed:
					//Leave exclusive fullscreen if needed
					if(mode == Mode::Fullscreen) [impl->mac->win.contentView exitFullScreenModeWithOptions:nil];

					//Restore presentation options if needed
					if(mode == Mode::Borderless) [impl->mac->app setPresentationOptions:impl->mac->lastPresentOpts];

					//Let window be obscured
					[impl->mac->win setLevel:NSNormalWindowLevel];

					//Set style
					[impl->mac->win setStyleMask:windowedStyle];

					//Apply last position and size
					@autoreleasepool {
						NSRect frame = NSMakeRect(lastPos.x, lastPos.y, lastSize.x, lastSize.y);
						[impl->mac->win setFrame:frame display:YES];
					}
					size = lastSize;
					break;

				case Mode::Borderless:
					@autoreleasepool {
						//Put window in foreground and keep it there
						[impl->mac->win orderFrontRegardless];
						[impl->mac->win setLevel:NSFloatingWindowLevel];

						//Hide menubar and dock
						impl->mac->lastPresentOpts = [impl->mac->app presentationOptions];
						[impl->mac->app setPresentationOptions:(NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar)];

						//Set style
						[impl->mac->win setStyleMask:(NSWindowStyleMaskBorderless)];

						//Set window size to screen size
						NSRect screenFrame = [impl->mac->win frameRectForContentRect:scr.frame];
						[impl->mac->win setFrame:screenFrame display:YES animate:YES];
						size = {screenFrame.size.width, screenFrame.size.height};
					}
					break;

				case Mode::Fullscreen:
					@autoreleasepool {
						//Put window in foreground
						[impl->mac->win orderFrontRegardless];

						//Enter fullscreen
						NSDictionary<NSViewFullScreenModeOptionKey, id>* opts = @{
							NSFullScreenModeApplicationPresentationOptions : @(NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar | NSApplicationPresentationDisableProcessSwitching | NSApplicationPresentationFullScreen | NSApplicationPresentationDisableHideApplication)
						};
						Check<ExternalException>([impl->mac->win.contentView enterFullScreenMode:scr withOptions:opts], "Failed to enter fullscreen mode!");
					}
					break;
			}
		}

		mode = newMode;

		DataEvent<glm::uvec2> wre("Window", size);
		EventManager::Get().Dispatch(wre);
	}
}