#pragma once

#include <memory>

namespace Cacao {
	class Context {
	  public:
		Context();
		~Context();

		void SetVSync(bool vsync);
		void SwapBuffers();

		struct Impl;

	  private:
		std::unique_ptr<Impl> impl;
	};

	inline std::shared_ptr<Context> ctx;
}