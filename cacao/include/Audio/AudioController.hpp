#pragma once

#include "Sound.hpp"

namespace Cacao {
	//Controls the playback of audio
	class AudioController {
	  public:
		//Get the instance or create one if it doesn't exist.
		static AudioController* GetInstance();

		//Start the controller
		void Start();

		//Stop the controller
		void Stop();

	  private:
		//Singleton members
		static AudioController* instance;
		static bool instanceExists;

		//Run the tick controller
		void Run(std::stop_token stopTkn);

		bool isRunning;

		std::jthread* thread;

		AudioController() {}
	}
}
