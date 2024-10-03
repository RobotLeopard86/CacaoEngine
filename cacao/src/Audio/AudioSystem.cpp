#include "Audio/AudioSystem.hpp"

#include "Core/Exception.hpp"
#include "Events/EventSystem.hpp"

#include "AL/alext.h"

namespace Cacao {
	//Required static variable initialization
	AudioSystem* AudioSystem::instance = nullptr;
	bool AudioSystem::instanceExists = false;

	//Singleton accessor
	AudioSystem* AudioSystem::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new AudioSystem();
			instanceExists = true;
		}

		return instance;
	}

	void AudioSystem::Init() {
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot initialize the initialized audio system!")

		//Initialize OpenAL-Soft
		dev = alcOpenDevice(nullptr);
		CheckException(dev, Exception::GetExceptionCodeFromMeaning("External"), "Failed to open the OpenAL device!")
		ctx = alcCreateContext(dev, nullptr);

		//Make audio context current
		alcMakeContextCurrent(ctx);

		isInitialized = true;

		//Set initial global gain
		SetGlobalGain(1.0f);
	}

	void AudioSystem::Shutdown() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot shutdown the uninitialized audio system!")

		//Alert all audio objects that it is shutdown time
		Event e("AudioShutdown");
		EventManager::GetInstance()->DispatchSignaled(e)->WaitAll();

		isInitialized = false;

		//Shutdown OpenAL-Soft
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(ctx);
		alcCloseDevice(dev);
	}

	void AudioSystem::SetGlobalGain(float value) {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to set the global gain!")

		alListenerf(AL_GAIN, value);
	}

	float AudioSystem::GetGlobalGain() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get the global gain!")

		float retval;
		alGetListenerf(AL_GAIN, &retval);
		return retval;
	}
}
