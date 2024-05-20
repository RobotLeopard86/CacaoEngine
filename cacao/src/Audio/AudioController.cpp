#include "Audio/AudioController.hpp"

#include "Core/Exception.hpp"

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

	void AudioController::Start() {
		CheckException(!isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot start the already started dynamic tick controller!")
		isRunning = true;

		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(AudioController::Run));
	}

	void AudioController::Stop() {
		CheckException(isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot stop the unstarted dynamic tick controller!")
		//Stop run thread
		thread->request_stop();
		thread->join();

		//Delete thread object
		delete thread;
		thread = nullptr;

		isRunning = false;
	}

	void AudioController::Run(std::stop_token stopTkn) {
		Init();
		RunImpl(stopTkn);
		Shutdown();
	}
}
