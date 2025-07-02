#pragma once

#include "Cacao/Window.hpp"
#include "WindowImplBase.hpp"

#include <AppKit/AppKit.h>
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
	class MacOSWindowImpl : public Window::Impl {
	  public:
		void CreateWindow() override;
		void DestroyWindow() override;
		void HandleEvents() override;
		bool Minimized() override;
		const glm::uvec2 ContentAreaSize() override;
		void Visibility(bool visible) override;
		void Title(const std::string& title) override;
		void Resize(const glm::uvec2& size) override;
		void ModeChange(Window::Mode mode) override;
		void SaveWinPos() override;
		void SaveWinSize() override;
		void RestoreWin() override;

		const std::string ProviderID() const override {
			return "cocoa";
		}

		CacaoApp* app;
		CacaoAppDelegate* del;
		CacaoWinDelegate* wdel;
		NSWindow* win;
		NSApplicationPresentationOptions lastPresentOpts;
	};
}