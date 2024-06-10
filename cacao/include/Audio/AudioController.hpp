#pragma once

#include "Sound.hpp"
#include "World/Entity.hpp"
#include "Utilities/Task.hpp"

#include <queue>
#include <mutex>

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

		//Run something on the audio thread
		std::shared_future<void> RunAudioThreadJob(std::function<void()> job);

		//Set the entity to be used as a reference for 3D audio spatialization
		//This maintains a non-owning reference, so if the entity is freed then 3D audio
		//will be played from the last known position and rotation
		void Set3DAudioTarget(std::weak_ptr<Entity> entity);

		//Returns if a 3D audio target has been set and is still alive
		//Should the entity be freed this function will return false
		bool Has3DAudioTarget() {
			return has3DAudioTarget && !target3DAudio.expired();
		}

		bool IsAudioSystemInitialized() {
			return isInitialized;
		}

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
		bool isInitialized;

		std::jthread* thread;

		//3D audio stuff
		std::weak_ptr<Entity> target3DAudio;
		bool has3DAudioTarget;

		//Audio job queue
		std::queue<Task> audioJobs;
		std::mutex jobQueueMutex;

		AudioController()
		  : isRunning(false), isInitialized(false), thread(nullptr) {}
	};
}
