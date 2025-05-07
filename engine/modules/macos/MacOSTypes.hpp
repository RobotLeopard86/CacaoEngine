#pragma once

#include "Cacao/Window.hpp"
#include <memory>

#import <Cocoa/Cocoa.h>

using namespace Cacao;

@interface CacaoAppDelegate : NSObject <NSApplicationDelegate>
@end

namespace Cacao {
	struct MacOSCommon {
		NSWindow* win;
		CacaoAppDelegate* del;
	};

	struct Window::Impl {
		std::unique_ptr<MacOSCommon> mac;
	};
}