#include "Pipeline/RenderPhase.hpp"
#include "Graphics/Window.hpp"

namespace Citrus {
	//Required static variable initialization
	RenderPhase* RenderPhase::instance = nullptr;
	bool RenderPhase::instanceExists = false;

	//Singleton accessor
	RenderPhase* RenderPhase::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new RenderPhase();
			instanceExists = true;
		}

		return instance;
	}

	void RenderPhase::_Run(){
		if(!renderQueue.empty()){

		}
		//Update window
		Window::GetInstance()->Update();
	}
}
