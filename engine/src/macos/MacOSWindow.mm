#include "Cacao/Exceptions.hpp"
#include "Cacao/Input.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/EventManager.hpp"
#include "ImplAccessor.hpp"
#import "MacOSTypes.hpp"
#include <CoreFoundation/CFRunLoop.h>

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

#include <memory>

#include "macos-keycodes.h"
#include "eternal.hpp"

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
			win = [[CacaoWin alloc] initWithContentRect:frame styleMask:windowedStyle backing:NSBackingStoreBuffered defer:NO];
			win.layoutMgr = [[NSLayoutManager alloc] init];
			[win setTitle:[[NSString alloc] initWithCString:title.c_str() encoding:[NSString defaultCStringEncoding]]];
			[win setIsVisible:visible];
			[win setDelegate:wdel];
			[win setAcceptsMouseMovedEvents:YES];
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
				//Handle OS stuff
				[app sendEvent:event];
				[app updateWindows];
				CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, false);
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
		constexpr const static auto codes = mapbox::eternal::map<unsigned int, unsigned int>({{kVK_Return, CACAO_KEY_ENTER},
			{kVK_Escape, CACAO_KEY_ESCAPE},
			{kVK_Delete, CACAO_KEY_BACKSPACE},
			{kVK_Tab, CACAO_KEY_TAB},
			{kVK_Space, CACAO_KEY_SPACE},
			{kVK_ANSI_Quote, CACAO_KEY_APOSTROPHE},
			{kVK_ANSI_Comma, CACAO_KEY_COMMA},
			{kVK_ANSI_Minus, CACAO_KEY_MINUS},
			{kVK_ANSI_Equal, CACAO_KEY_EQUALS},
			{kVK_ANSI_Period, CACAO_KEY_PERIOD},
			{kVK_ANSI_Slash, CACAO_KEY_SLASH},
			{kVK_ANSI_0, CACAO_KEY_0},
			{kVK_ANSI_1, CACAO_KEY_1},
			{kVK_ANSI_2, CACAO_KEY_2},
			{kVK_ANSI_3, CACAO_KEY_3},
			{kVK_ANSI_4, CACAO_KEY_4},
			{kVK_ANSI_5, CACAO_KEY_5},
			{kVK_ANSI_6, CACAO_KEY_6},
			{kVK_ANSI_7, CACAO_KEY_7},
			{kVK_ANSI_8, CACAO_KEY_8},
			{kVK_ANSI_9, CACAO_KEY_9},
			{kVK_ANSI_Semicolon, CACAO_KEY_SEMICOLON},
			{kVK_ANSI_LeftBracket, CACAO_KEY_LEFT_BRACKET},
			{kVK_ANSI_RightBracket, CACAO_KEY_RIGHT_BRACKET},
			{kVK_ANSI_Backslash, CACAO_KEY_BACKSLASH},
			{kVK_ANSI_Grave, CACAO_KEY_GRAVE_ACCENT},
			{kVK_ANSI_A, CACAO_KEY_A},
			{kVK_ANSI_B, CACAO_KEY_B},
			{kVK_ANSI_C, CACAO_KEY_C},
			{kVK_ANSI_D, CACAO_KEY_D},
			{kVK_ANSI_E, CACAO_KEY_E},
			{kVK_ANSI_F, CACAO_KEY_F},
			{kVK_ANSI_G, CACAO_KEY_G},
			{kVK_ANSI_H, CACAO_KEY_H},
			{kVK_ANSI_I, CACAO_KEY_I},
			{kVK_ANSI_J, CACAO_KEY_J},
			{kVK_ANSI_K, CACAO_KEY_K},
			{kVK_ANSI_L, CACAO_KEY_L},
			{kVK_ANSI_M, CACAO_KEY_M},
			{kVK_ANSI_N, CACAO_KEY_N},
			{kVK_ANSI_O, CACAO_KEY_O},
			{kVK_ANSI_P, CACAO_KEY_P},
			{kVK_ANSI_Q, CACAO_KEY_Q},
			{kVK_ANSI_R, CACAO_KEY_R},
			{kVK_ANSI_S, CACAO_KEY_S},
			{kVK_ANSI_T, CACAO_KEY_T},
			{kVK_ANSI_U, CACAO_KEY_U},
			{kVK_ANSI_V, CACAO_KEY_V},
			{kVK_ANSI_W, CACAO_KEY_W},
			{kVK_ANSI_X, CACAO_KEY_X},
			{kVK_ANSI_Y, CACAO_KEY_Y},
			{kVK_ANSI_Z, CACAO_KEY_Z},
			{kVK_CapsLock, CACAO_KEY_CAPS_LOCK},
			{kVK_F1, CACAO_KEY_F1},
			{kVK_F2, CACAO_KEY_F2},
			{kVK_F3, CACAO_KEY_F3},
			{kVK_F4, CACAO_KEY_F4},
			{kVK_F5, CACAO_KEY_F5},
			{kVK_F6, CACAO_KEY_F6},
			{kVK_F7, CACAO_KEY_F7},
			{kVK_F8, CACAO_KEY_F8},
			{kVK_F9, CACAO_KEY_F9},
			{kVK_F10, CACAO_KEY_F10},
			{kVK_F11, CACAO_KEY_F11},
			{kVK_F12, CACAO_KEY_F12},
			{kVK_ForwardDelete, CACAO_KEY_DELETE},
			{kVK_Home, CACAO_KEY_HOME},
			{kVK_PageUp, CACAO_KEY_PAGE_UP},
			{kVK_End, CACAO_KEY_END},
			{kVK_PageDown, CACAO_KEY_PAGE_DOWN},
			{kVK_RightArrow, CACAO_KEY_RIGHT},
			{kVK_LeftArrow, CACAO_KEY_LEFT},
			{kVK_DownArrow, CACAO_KEY_DOWN},
			{kVK_UpArrow, CACAO_KEY_UP},
			{kVK_ANSI_KeypadDivide, CACAO_KEY_KP_DIVIDE},
			{kVK_ANSI_KeypadMultiply, CACAO_KEY_KP_MULTIPLY},
			{kVK_ANSI_KeypadMinus, CACAO_KEY_KP_MINUS},
			{kVK_ANSI_KeypadPlus, CACAO_KEY_KP_PLUS},
			{kVK_ANSI_KeypadEnter, CACAO_KEY_KP_ENTER},
			{kVK_ANSI_Keypad1, CACAO_KEY_KP_1},
			{kVK_ANSI_Keypad2, CACAO_KEY_KP_2},
			{kVK_ANSI_Keypad3, CACAO_KEY_KP_3},
			{kVK_ANSI_Keypad4, CACAO_KEY_KP_4},
			{kVK_ANSI_Keypad5, CACAO_KEY_KP_5},
			{kVK_ANSI_Keypad6, CACAO_KEY_KP_6},
			{kVK_ANSI_Keypad7, CACAO_KEY_KP_7},
			{kVK_ANSI_Keypad8, CACAO_KEY_KP_8},
			{kVK_ANSI_Keypad9, CACAO_KEY_KP_9},
			{kVK_ANSI_Keypad0, CACAO_KEY_KP_0},
			{kVK_ANSI_KeypadDecimal, CACAO_KEY_KP_PERIOD},
			{kVK_Control, CACAO_KEY_LEFT_CONTROL},
			{kVK_Shift, CACAO_KEY_LEFT_SHIFT},
			{kVK_Option, CACAO_KEY_LEFT_ALT},
			{kVK_Command, CACAO_KEY_LEFT_SUPER},
			{kVK_RightControl, CACAO_KEY_RIGHT_CONTROL},
			{kVK_RightShift, CACAO_KEY_RIGHT_SHIFT},
			{kVK_RightOption, CACAO_KEY_RIGHT_ALT},
			{kVK_RightCommand, CACAO_KEY_RIGHT_SUPER}});
		if(codes.contains(key)) return codes.at(key);
		return key;
	}

	unsigned int MacOSWindowImpl::ConvertButtonCode(unsigned int button) {
		//macOS doesn't use this
		return button;
	}
}