#pragma once

#include "Cacao/Window.hpp"

#import <Cocoa/Cocoa.h>
#include <memory>

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