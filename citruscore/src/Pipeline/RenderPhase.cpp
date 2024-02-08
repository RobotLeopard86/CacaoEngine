#include "Pipeline/RenderPhase.hpp"

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
}
