#include "Control/DynTickController.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Utilities/MiscUtils.hpp"

#include <sstream>

namespace Cacao {
	//Required static variable initialization
	DynTickController* DynTickController::instance = nullptr;
	bool DynTickController::instanceExists = false;

	//Singleton accessor
	DynTickController* DynTickController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new DynTickController();
			instanceExists = true;
			instance->isRunning = false;
		}

		return instance;
	}

	void DynTickController::Start(){
		if(isRunning) {
			Logging::EngineLog("Cannot start the already started dynamic tick controller!", LogLevel::Error);
			return;
		}
		isRunning = true;
		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(DynTickController::Run));
	}

	void DynTickController::Stop(){
		if(!isRunning) {
			Logging::EngineLog("Cannot stop the not started dynamic tick controller!", LogLevel::Error);
			return;
		}
		//Stop run thread
		thread->request_stop();
		thread->join();

		//Delete thread object
		delete thread;
		thread = nullptr;

		isRunning = false;
	}

	void DynTickController::Run(std::stop_token stopTkn) {
		//Run while we haven't been asked to stop
		while(!stopTkn.stop_requested()){
			//Get time at tick start and calculate ideal run time
			std::chrono::steady_clock::time_point tickStart = std::chrono::steady_clock::now();
			std::chrono::steady_clock::time_point idealStopTime = tickStart + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(1)/Engine::GetInstance()->cfg.targetDynTPS);
			
			//Check elapsed time
			std::chrono::steady_clock::time_point tickEnd = std::chrono::steady_clock::now();
			
			//If we stopped before the ideal max time, wait until that point
			//Otherwise, run the next tick immediately
			if(tickEnd < idealStopTime) std::this_thread::sleep_for(idealStopTime - tickEnd);
		}
	}
}
