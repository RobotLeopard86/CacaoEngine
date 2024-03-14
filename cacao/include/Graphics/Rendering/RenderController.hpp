#pragma once

#include <queue>

namespace Cacao {
	//Controller for rendering frames
	class RenderController {
	public:
		//Get the instance or create one if it doesn't exist.
		static RenderController* GetInstance();
	private:
		//Singleton members
		static RenderController* instance;
		static bool instanceExists;

		RenderController() {}
	};
}
