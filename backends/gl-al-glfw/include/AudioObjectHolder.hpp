#pragma once

#include "alure2.h"

namespace Cacao {
	//Owns the core objects necessary for audio
	class AudioObjectHolder {
	  public:
		//Get the instance or create one if it doesn't exist.
		static AudioObjectHolder* GetInstance();

		//Audio device
		alure::Device dev;

		//Audio context
		alure::Context ctx;

	  private:
		//Singleton members
		static AudioObjectHolder* instance;
		static bool instanceExists;
	};
}

//Useful defines for ease of access
#define audioDev AudioObjectHolder::GetInstance()->dev
#define audioCtx AudioObjectHolder::GetInstance()->ctx