#include "Audio/AudioController.hpp"
#include "AudioObjectHolder.hpp"
#include "Events/EventSystem.hpp"

#include "alure2.h"

#include <string>

namespace Cacao {
	//Static member initialization
	AudioObjectHolder* AudioObjectHolder::instance = nullptr;
	bool AudioObjectHolder::instanceExists = false;

	//Singleton accessor
	AudioObjectHolder* AudioObjectHolder::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new AudioObjectHolder();
			instanceExists = true;
		}

		return instance;
	}

	void AudioController::Init() {
		//Create the device and context
		audioDev = alure::DeviceManager::getInstance().openPlayback();
		Logging::EngineLog(std::string("Opened audio device \"") + audioDev.getName() + std::string("\" for playback!"));
		audioCtx = audioDev.createContext();

		//Set the context as current
		alure::Context::MakeCurrent(audioCtx);
	}

	void AudioController::RunImpl(std::stop_token& stopTkn) {
		while(!stopTkn.stop_requested()) {
			audioCtx.update();
		}
	}

	void AudioController::Shutdown() {
		//Signal all audio assets to finish context operations as it will be destroyed
		Event destructionSignal = Event {"AudioContextDestruction"};
		EventManager::GetInstance()->DispatchSignaled(destructionSignal)->wait();

		//Unset context
		alure::Context::MakeCurrent(nullptr);

		//Destroy context and close device
		audioCtx.destroy();
		audioDev.close();

		//Delete audio object holder to free memory
		delete AudioObjectHolder::GetInstance();
	}
}