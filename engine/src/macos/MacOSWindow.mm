#include "Cacao/Event.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Input.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/EventManager.hpp"
#include "ImplAccessor.hpp"
#import "MacOSTypes.hpp"
#include "glm/fwd.hpp"
#include <AppKit/AppKit.h>
#include <GameController/GCKeyCodes.h>
#include <GameController/GameController.h>
#include <Foundation/Foundation.h>

#include <memory>

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
			NSString* appName = [[NSString alloc] initWithUTF8String:Engine::Get().GetInitConfig().clientID.displayName.c_str()];
			[[NSProcessInfo processInfo] setProcessName:appName];
			NSMenu* mainMenu = [[NSMenu alloc] init];
			NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:appName action:nil keyEquivalent:@""];
			NSMenu* appMenu = [[NSMenu alloc] init];

			NSMenuItem* aboutMenuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"About %@", appName]
																   action:@selector(orderFrontStandardAboutPanel:)
															keyEquivalent:@""];
			[aboutMenuItem setTarget:app];
			[appMenu addItem:aboutMenuItem];
			[appMenu addItem:[NSMenuItem separatorItem]];

			NSMenu* servicesMenu = [[NSMenu alloc] init];
			[app setServicesMenu:servicesMenu];
			NSMenuItem* servicesMenuItem = [[NSMenuItem alloc] initWithTitle:@"Services" action:nil keyEquivalent:@""];
			[servicesMenuItem setSubmenu:servicesMenu];
			[servicesMenuItem setTarget:app];
			[appMenu addItem:servicesMenuItem];
			[appMenu addItem:[NSMenuItem separatorItem]];

			NSMenuItem* hideMenuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
																  action:@selector(hide:)
														   keyEquivalent:@"h"];
			[hideMenuItem setTarget:app];
			[appMenu addItem:hideMenuItem];
			NSMenuItem* hideOthersMenuItem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
																		action:@selector(hideOtherApplications:)
																 keyEquivalent:@"h"];
			[hideOthersMenuItem setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
			[hideOthersMenuItem setTarget:app];
			[appMenu addItem:hideOthersMenuItem];
			NSMenuItem* showAllMenuItem = [[NSMenuItem alloc] initWithTitle:@"Show All"
																	 action:@selector(unhideAllApplications:)
															  keyEquivalent:@""];
			[showAllMenuItem setTarget:app];
			[appMenu addItem:showAllMenuItem];
			[appMenu addItem:[NSMenuItem separatorItem]];

			NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
																  action:@selector(terminate:)
														   keyEquivalent:@"q"];
			[quitMenuItem setTarget:app];
			[appMenu addItem:quitMenuItem];

			[appMenuItem setSubmenu:appMenu];
			[mainMenu addItem:appMenuItem];
			[app setMainMenu:mainMenu];
			[[mainMenu itemAtIndex:0] setTitle:appName];

			//Make frame
			NSRect frame = NSMakeRect(0, 0, size.x, size.y);

			//Create window and delegate
			wdel = [[CacaoWinDelegate alloc] init];
			win = [[CacaoWin alloc] initWithContentRect:frame styleMask:windowedStyle backing:NSBackingStoreBuffered defer:NO];
			[win setTitle:[[NSString alloc] initWithCString:title.c_str() encoding:[NSString defaultCStringEncoding]]];
			[win setIsVisible:visible];
			[win setDelegate:wdel];
			[win makeFirstResponder:win];
			[win setAcceptsMouseMovedEvents:YES];
			[win makeKeyAndOrderFront:nil];

			//Connect graphics
			PAL::Get().GfxConnect();

			//Finish launching
			[app activateIgnoringOtherApps:YES];
			[app finishLaunching];

			//Start app loop
			[app run];
		}
	}

	void MacOSWindowImpl::DestroyWindow() {
		//Disconnect graphics
		PAL::Get().GfxDisconnect();

		//Close window and release objects
		[keyInput release];
		[keyboard release];
		[win close];
		[wdel release];
		[del release];
		[app release];
	}

	void MacOSWindowImpl::HandleEvents() {
		@autoreleasepool {
			//Pump Cocoa events
			NSEvent* event;
			while((event = [app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate dateWithTimeIntervalSinceNow:0.001] inMode:NSDefaultRunLoopMode dequeue:YES])) {
				//Handle OS stuff
				[app sendEvent:event];
				[app updateWindows];
			}

			//System runloop
			[[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.001]];
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
		const static auto codes = mapbox::eternal::map<GCKeyCode, unsigned int>({{GCKeyCodeReturnOrEnter, CACAO_KEY_ENTER},
			{GCKeyCodeEscape, CACAO_KEY_ESCAPE},
			{GCKeyCodeDeleteOrBackspace, CACAO_KEY_BACKSPACE},
			{GCKeyCodeTab, CACAO_KEY_TAB},
			{GCKeyCodeSpacebar, CACAO_KEY_SPACE},
			{GCKeyCodeQuote, CACAO_KEY_APOSTROPHE},
			{GCKeyCodeComma, CACAO_KEY_COMMA},
			{GCKeyCodeHyphen, CACAO_KEY_MINUS},
			{GCKeyCodeEqualSign, CACAO_KEY_EQUALS},
			{GCKeyCodePeriod, CACAO_KEY_PERIOD},
			{GCKeyCodeSlash, CACAO_KEY_SLASH},
			{GCKeyCodeZero, CACAO_KEY_0},
			{GCKeyCodeOne, CACAO_KEY_1},
			{GCKeyCodeTwo, CACAO_KEY_2},
			{GCKeyCodeThree, CACAO_KEY_3},
			{GCKeyCodeFour, CACAO_KEY_4},
			{GCKeyCodeFive, CACAO_KEY_5},
			{GCKeyCodeSix, CACAO_KEY_6},
			{GCKeyCodeSeven, CACAO_KEY_7},
			{GCKeyCodeEight, CACAO_KEY_8},
			{GCKeyCodeNine, CACAO_KEY_9},
			{GCKeyCodeSemicolon, CACAO_KEY_SEMICOLON},
			{GCKeyCodeOpenBracket, CACAO_KEY_LEFT_BRACKET},
			{GCKeyCodeCloseBracket, CACAO_KEY_RIGHT_BRACKET},
			{GCKeyCodeBackslash, CACAO_KEY_BACKSLASH},
			{GCKeyCodeGraveAccentAndTilde, CACAO_KEY_GRAVE_ACCENT},
			{GCKeyCodeKeyA, CACAO_KEY_A},
			{GCKeyCodeKeyB, CACAO_KEY_B},
			{GCKeyCodeKeyC, CACAO_KEY_C},
			{GCKeyCodeKeyD, CACAO_KEY_D},
			{GCKeyCodeKeyE, CACAO_KEY_E},
			{GCKeyCodeKeyF, CACAO_KEY_F},
			{GCKeyCodeKeyG, CACAO_KEY_G},
			{GCKeyCodeKeyH, CACAO_KEY_H},
			{GCKeyCodeKeyI, CACAO_KEY_I},
			{GCKeyCodeKeyJ, CACAO_KEY_J},
			{GCKeyCodeKeyK, CACAO_KEY_K},
			{GCKeyCodeKeyL, CACAO_KEY_L},
			{GCKeyCodeKeyM, CACAO_KEY_M},
			{GCKeyCodeKeyN, CACAO_KEY_N},
			{GCKeyCodeKeyO, CACAO_KEY_O},
			{GCKeyCodeKeyP, CACAO_KEY_P},
			{GCKeyCodeKeyQ, CACAO_KEY_Q},
			{GCKeyCodeKeyR, CACAO_KEY_R},
			{GCKeyCodeKeyS, CACAO_KEY_S},
			{GCKeyCodeKeyT, CACAO_KEY_T},
			{GCKeyCodeKeyU, CACAO_KEY_U},
			{GCKeyCodeKeyV, CACAO_KEY_V},
			{GCKeyCodeKeyW, CACAO_KEY_W},
			{GCKeyCodeKeyX, CACAO_KEY_X},
			{GCKeyCodeKeyY, CACAO_KEY_Y},
			{GCKeyCodeKeyZ, CACAO_KEY_Z},
			{GCKeyCodeCapsLock, CACAO_KEY_CAPS_LOCK},
			{GCKeyCodeF1, CACAO_KEY_F1},
			{GCKeyCodeF2, CACAO_KEY_F2},
			{GCKeyCodeF3, CACAO_KEY_F3},
			{GCKeyCodeF4, CACAO_KEY_F4},
			{GCKeyCodeF5, CACAO_KEY_F5},
			{GCKeyCodeF6, CACAO_KEY_F6},
			{GCKeyCodeF7, CACAO_KEY_F7},
			{GCKeyCodeF8, CACAO_KEY_F8},
			{GCKeyCodeF9, CACAO_KEY_F9},
			{GCKeyCodeF10, CACAO_KEY_F10},
			{GCKeyCodeF11, CACAO_KEY_F11},
			{GCKeyCodeF12, CACAO_KEY_F12},
			{GCKeyCodePrintScreen, CACAO_KEY_PRINT_SCREEN},
			{GCKeyCodeScrollLock, CACAO_KEY_SCROLL_LOCK},
			{GCKeyCodePause, CACAO_KEY_PAUSE},
			{GCKeyCodeInsert, CACAO_KEY_INSERT},
			{GCKeyCodeDeleteForward, CACAO_KEY_DELETE},
			{GCKeyCodeHome, CACAO_KEY_HOME},
			{GCKeyCodePageUp, CACAO_KEY_PAGE_UP},
			{GCKeyCodeEnd, CACAO_KEY_END},
			{GCKeyCodePageDown, CACAO_KEY_PAGE_DOWN},
			{GCKeyCodeRightArrow, CACAO_KEY_RIGHT},
			{GCKeyCodeLeftArrow, CACAO_KEY_LEFT},
			{GCKeyCodeDownArrow, CACAO_KEY_DOWN},
			{GCKeyCodeUpArrow, CACAO_KEY_UP},
			{GCKeyCodeKeypadNumLock, CACAO_KEY_NUM_LOCK},
			{GCKeyCodeKeypadSlash, CACAO_KEY_KP_DIVIDE},
			{GCKeyCodeKeypadAsterisk, CACAO_KEY_KP_MULTIPLY},
			{GCKeyCodeKeypadHyphen, CACAO_KEY_KP_MINUS},
			{GCKeyCodeKeypadPlus, CACAO_KEY_KP_PLUS},
			{GCKeyCodeKeypadEnter, CACAO_KEY_KP_ENTER},
			{GCKeyCodeKeypad1, CACAO_KEY_KP_1},
			{GCKeyCodeKeypad2, CACAO_KEY_KP_2},
			{GCKeyCodeKeypad3, CACAO_KEY_KP_3},
			{GCKeyCodeKeypad4, CACAO_KEY_KP_4},
			{GCKeyCodeKeypad5, CACAO_KEY_KP_5},
			{GCKeyCodeKeypad6, CACAO_KEY_KP_6},
			{GCKeyCodeKeypad7, CACAO_KEY_KP_7},
			{GCKeyCodeKeypad8, CACAO_KEY_KP_8},
			{GCKeyCodeKeypad9, CACAO_KEY_KP_9},
			{GCKeyCodeKeypad0, CACAO_KEY_KP_0},
			{GCKeyCodeKeypadPeriod, CACAO_KEY_KP_PERIOD},
			{GCKeyCodeLeftControl, CACAO_KEY_LEFT_CONTROL},
			{GCKeyCodeLeftShift, CACAO_KEY_LEFT_SHIFT},
			{GCKeyCodeLeftAlt, CACAO_KEY_LEFT_ALT},
			{GCKeyCodeLeftGUI, CACAO_KEY_LEFT_SUPER},
			{GCKeyCodeRightControl, CACAO_KEY_RIGHT_CONTROL},
			{GCKeyCodeRightShift, CACAO_KEY_RIGHT_SHIFT},
			{GCKeyCodeRightAlt, CACAO_KEY_RIGHT_ALT},
			{GCKeyCodeRightGUI, CACAO_KEY_RIGHT_SUPER}});
		if(codes.contains(key)) return codes.at(key);
		return key;
	}

	unsigned int MacOSWindowImpl::ConvertButtonCode(unsigned int button) {
		//macOS doesn't use this
		return button;
	}

	void MacOSWindowImpl::ConfigureKeyboard(GCKeyboard* kb) {
		//Register keyboard handler
		keyboard = kb;
		keyInput = kb.keyboardInput;
		[keyInput setKeyChangedHandler:^(GCKeyboardInput*, GCControllerButtonInput*, GCKeyCode keyCode, BOOL pressed) {
			DataEvent<unsigned int> keyEvent(pressed == YES ? "KeyDown" : "KeyUp", ConvertKeycode(keyCode));
			EventManager::Get().Dispatch(keyEvent);
		}];
	}
}

static std::atomic_bool quitRequested;

using namespace Cacao;

@implementation CacaoAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
	//Post event to make sure everything is working
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

	//Setup keyboard
	[[NSNotificationCenter defaultCenter] addObserverForName:GCKeyboardDidConnectNotification
													  object:nil
													   queue:nil
												  usingBlock:^(NSNotification* notification) {
													  WIN_IMPL(MacOS).ConfigureKeyboard(notification.object);
												  }];

	//Setup mouse
	auto handler = ^NSEvent*(NSEvent* event) {
		switch(event.type) {
			case NSEventTypeMouseMoved: {
				DataEvent<glm::dvec2> mme("MouseMove", glm::dvec2 {event.locationInWindow.x, event.locationInWindow.y});
				EventManager::Get().Dispatch(mme);
				break;
			}
			case NSEventTypeScrollWheel: {
				DataEvent<glm::dvec2> mse("MouseScroll", glm::dvec2 {event.scrollingDeltaX, event.scrollingDeltaY});
				EventManager::Get().Dispatch(mse);
				break;
			}
			case NSEventTypeLeftMouseDown: {
				DataEvent<unsigned int> btnEvent("MousePress", CACAO_BUTTON_LEFT);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			case NSEventTypeLeftMouseUp: {
				DataEvent<unsigned int> btnEvent("MouseRelease", CACAO_BUTTON_LEFT);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			case NSEventTypeRightMouseDown: {
				DataEvent<unsigned int> btnEvent("MousePress", CACAO_BUTTON_RIGHT);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			case NSEventTypeRightMouseUp: {
				DataEvent<unsigned int> btnEvent("MouseRelease", CACAO_BUTTON_RIGHT);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			//Generally, most mice don't have a ton of non-specialty buttons (and the specialty ones are usually overriden by other software)
			//So, we assume buttons that aren't left or right are middle.
			case NSEventTypeOtherMouseDown: {
				DataEvent<unsigned int> btnEvent("MousePress", CACAO_BUTTON_MIDDLE);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			case NSEventTypeOtherMouseUp: {
				DataEvent<unsigned int> btnEvent("MouseRelease", CACAO_BUTTON_MIDDLE);
				EventManager::Get().Dispatch(btnEvent);
				break;
			}
			default:
				break;
		}
		return event;
	};
	[NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMouseMoved |
												  NSEventMaskScrollWheel |
												  NSEventMaskLeftMouseDown |
												  NSEventMaskLeftMouseUp |
												  NSEventMaskRightMouseDown |
												  NSEventMaskRightMouseUp |
												  NSEventMaskOtherMouseDown |
												  NSEventMaskOtherMouseUp
										  handler:handler];

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

//This only exists to silence AppKit "bonk" sounds since we use GCKeyboard instead
@implementation CacaoWin

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)performKeyEquivalent:(NSEvent*)event {
	return YES;
}

- (void)keyDown:(NSEvent*)event {
}
- (void)keyUp:(NSEvent*)event {
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