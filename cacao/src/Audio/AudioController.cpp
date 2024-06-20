#include "Audio/AudioController.hpp"

#include "Core/Exception.hpp"
#include "Events/EventSystem.hpp"

#include "AL/alext.h"

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

	void AudioController::SetGlobalGain(float value) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to set the global gain!")

		alListenerf(AL_GAIN, value);
	}

	float AudioController::GetGlobalGain() {
		float retval;
		alGetListenerf(AL_GAIN, &retval);
		return retval;
	}

	void
	AudioController::Run(std::stop_token stopTkn) {
		//Initialize OpenAL-Soft
		dev = alcOpenDevice(NULL);
		CheckException(dev, Exception::GetExceptionCodeFromMeaning("External"), "Failed to open the OpenAL device!")
		ctx = alcCreateContext(dev, NULL);

		//Make audio context current
		alcMakeContextCurrent(ctx);

		isInitialized = true;

		//Set initial global gain
		SetGlobalGain(1.0f);

		while(!stopTkn.stop_requested()) {
			//Update the 3D audio target if it exists
			if(has3DAudioTarget) {
				//Make sure that the target still exists
				if(target3DAudio.expired()) {
					has3DAudioTarget = false;
				} else {
					//Get position and orientation
					std::shared_ptr<Entity> target = target3DAudio.lock();
					glm::vec3 targetPos = target->transform.GetPosition();
					Orientation targetRot = target->transform.GetRotation();
					glm::vec3 at {targetRot.pitch, targetRot.yaw, targetRot.roll};
					glm::vec3 up = Calculate3DVectors(targetRot).up;

					//Apply to OpenAL
					alListener3f(AL_POSITION, targetPos.x, targetPos.y, targetPos.z);
					ALfloat listenerOrientation[6] = {at.x, at.y, at.z, up.x, up.y, up.z};
					alListenerfv(AL_ORIENTATION, listenerOrientation);
				}
			}

			//Take a nap
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		//Alert all audio objects that it is shutdown time
		Event e("AudioShutdown");
		EventManager::GetInstance()->DispatchSignaled(e)->WaitAll();

		isInitialized = false;

		//Shutdown OpenAL-Soft
		alcMakeContextCurrent(NULL);
		alcDestroyContext(ctx);
		alcCloseDevice(dev);
	}
}
