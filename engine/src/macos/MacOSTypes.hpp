#pragma once

#include "Cacao/Window.hpp"
#include <AppKit/AppKit.h>
#include <memory>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

using namespace Cacao;

@interface CacaoAppDelegate : NSObject <NSApplicationDelegate>
@end

@interface CacaoWinDelegate : NSObject <NSWindowDelegate>
@end

@interface CacaoApp : NSApplication
@end

namespace Cacao {
	struct MacOSCommon {
		CacaoApp* app;
		CacaoAppDelegate* del;
		CacaoWinDelegate* wdel;
		NSWindow* win;
		NSApplicationPresentationOptions lastPresentOpts;
	};

	struct Window::Impl {
		std::unique_ptr<MacOSCommon> mac;
	};
}