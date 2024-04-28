#pragma once

#include "Sound.hpp"

namespace Cacao {
	//Controls the playback of audio
	class AudioController {
	  public:
		//Get the instance or create one if it doesn't exist.
		static AudioController* GetInstance();

	  private:
		//Singleton members
		static AudioController* instance;
		static bool instanceExists;

		AudioController() {}
	}
}
