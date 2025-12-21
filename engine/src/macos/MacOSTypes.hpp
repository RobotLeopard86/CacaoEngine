#pragma once

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>

#include "Cacao/Window.hpp"
#include "impl/Window.hpp"

using namespace Cacao;

@interface CacaoAppDelegate : NSObject <NSApplicationDelegate>
@end

@interface CacaoWinDelegate : NSObject <NSWindowDelegate>
@end

@interface CacaoWin : NSWindow
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

		unsigned int ConvertKeycode(unsigned int key) override;
		unsigned int ConvertButtonCode(unsigned int button) override;

		const std::string ProviderID() const override {
			return "cocoa";
		}

		//macOS objects
		CacaoApp* app;
		CacaoAppDelegate* del;
		CacaoWinDelegate* wdel;
		CacaoWin* win;

		//Stored presentation options for mode transitions
		NSApplicationPresentationOptions lastPresentOpts;

		//Keyboard stuff
		//Keyboard
		GCKeyboard* keyboard;
		GCKeyboardInput* keyInput;
		void ConfigureKeyboard(GCKeyboard* kb);
	};
}