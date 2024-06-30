#pragma once

#include "Sound.hpp"
#include "World/Entity.hpp"
#include "Utilities/Task.hpp"

#include "AL/al.h"
#include "AL/alc.h"

#include <queue>
#include <mutex>

namespace Cacao {
	//Controls the playback of audio
	class AudioSystem {
	  public:
		//Get the instance or create one if it doesn't exist.
		static AudioSystem* GetInstance();

		//Initialize the audio system
		void Init();

		//Shut down the audio system
		void Shutdown();

		//Set the global gain (must be positive)
		void SetGlobalGain(float value);

		//Get the global gain value
		float GetGlobalGain();

		bool IsInitialized() {
			return isInitialized;
		}

	  private:
		//Singleton members
		static AudioSystem* instance;
		static bool instanceExists;

		bool isInitialized;

		//Audio context and device
		ALCdevice* dev;
		ALCcontext* ctx;

		AudioSystem()
		  : isInitialized(false) {}
	};
}
