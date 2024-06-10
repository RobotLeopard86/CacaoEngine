#include "Audio/AudioController.hpp"
#include "AudioObjectHolder.hpp"
#include "Audio/AudioPlayer.hpp"
#include "Events/EventSystem.hpp"
#include "Utilities/MiscUtils.hpp"

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
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't initialize already initialized audio controller!");

		//Create the device handle (from the system default device) and context
		audioDevMgr = alure::DeviceManager::getInstance();
		audioDev = audioDevMgr.openPlayback(audioDevMgr.defaultDeviceName(alure::DefaultDeviceType::Basic));
		audioCtx = audioDev.createContext();

		//Set the context as current
		alure::Context::MakeCurrent(audioCtx);

		//Set listener parameters
		audioCtx.getListener().setMetersPerUnit(1);
		audioCtx.setSpeedOfSound(343.3f);

		isInitialized = true;
	}

	void AudioController::RunImpl(std::stop_token& stopTkn) {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't run uninitialized audio controller!");

		while(!stopTkn.stop_requested()) {
			//Update 3D audio if we have a target
			if(has3DAudioTarget) {
				//Make sure that the target still exists
				if(target3DAudio.expired()) {
					has3DAudioTarget = false;
				} else {
					//Get position and orientation
					std::shared_ptr<Entity> target = target3DAudio.lock();
					glm::vec3 targetPos = target->transform.GetPosition();
					Orientation targetRot = target->transform.GetRotation();

					//Set position
					audioCtx.getListener().setPosition(alure::Vector3(targetPos.x, targetPos.y, targetPos.z));

					//Calculate and set orientation
					alure::Vector3 at(targetRot.pitch, targetRot.yaw, targetRot.roll);
					glm::vec3 upVec = Calculate3DVectors(targetRot).up;
					alure::Vector3 up(upVec.x, upVec.y, upVec.z);
					audioCtx.getListener().setOrientation({at, up});
				}
			}

			//Process the next audio job if there is one
			jobQueueMutex.lock();
			if(!audioJobs.empty()) {
				//Fetch the next task and unlock
				Task next = audioJobs.front();
				audioJobs.pop();
				jobQueueMutex.unlock();

				//Run the task
				try {
					//next.func();

					//Mark task as done
					next.status->set_value();
				} catch(...) {
					next.status->set_exception(std::current_exception());
				}
			} else {
				//Unlock immediately as there's nothing to do
				jobQueueMutex.unlock();
			}

			//Update audio context
			audioCtx.update();
		}
	}

	void AudioController::Shutdown() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't shutdown uninitialized audio controller!");

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

		isInitialized = false;
	}

	void AudioController::Set3DAudioTarget(std::weak_ptr<Entity> entity) {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't set 3D audio target of uninitialized audio controller!");
		CheckException(!entity.expired(), Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Can't set deleted entity as the 3D audio target!");

		target3DAudio = entity;
		has3DAudioTarget = true;
	}

	std::shared_future<void> AudioController::RunAudioThreadJob(std::function<void()> job) {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Can't run job on audio thread when audio controller is uninitialized!");

		//Acquire a lock on the job queue
		jobQueueMutex.lock();

		//Add the task to the queue
		Task task = audioJobs.emplace(job);

		//Release the lock
		jobQueueMutex.unlock();

		return task.status->get_future().share();
	}
}