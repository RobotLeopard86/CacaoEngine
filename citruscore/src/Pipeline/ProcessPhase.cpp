#include "Pipeline/ProcessPhase.hpp"

namespace Citrus {
	//Required static variable initialization
	ProcessPhase* ProcessPhase::instance = nullptr;
	bool ProcessPhase::instanceExists = false;

	//Singleton accessor
	ProcessPhase* ProcessPhase::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new ProcessPhase();
			instanceExists = true;
		}

		return instance;
	}
}
