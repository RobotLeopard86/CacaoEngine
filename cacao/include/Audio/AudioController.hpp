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

		//Run the controller
		void Run(std::stop_token stopTkn);

	  private:
		//Singleton members
		static AudioController* instance;
		static bool instanceExists;

		//Backend functions

		//Initialize the controller
		void Init();
		//Run the controller
		void RunImpl(std::stop_token& stopTkn);
		//Shutdown the controller
		void Shutdown();

		bool isRunning;

		std::jthread* thread;

		AudioController()
		  : isRunning(false), thread(nullptr) {}
	};
}
