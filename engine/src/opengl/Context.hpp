#pragma once

#include <memory>

namespace Cacao {
	class Context {
	  public:
		Context();
		~Context();

		//These two should be self-explanatory
		void SetVSync(bool vsync);
		void SwapBuffers();

		//These exist because setup happens on the main thread, but we need to use the context on the GPU thread
		//MakeCurrent sets the context active on the calling thread
		//Yield makes the context available on other threads
		void MakeCurrent();
		void Yield();

		struct Impl;

	  private:
		std::unique_ptr<Impl> impl;
	};

	inline std::shared_ptr<Context> ctx;
}