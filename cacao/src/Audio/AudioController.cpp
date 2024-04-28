#include "./Audio/AudioController.hpp"

namespace Cacao {
	//Required static variable initialization
	AudioController* AudioController::instance = nullptr;
	bool AudioController::instanceExists = false;

	//Singleton accessor
	AudioController* AudioController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new AudioController();
			instanceExists = true;
		}

		return instance;
	}
}
