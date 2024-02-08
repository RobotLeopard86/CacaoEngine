#pragma once

#include <future>

namespace Citrus {
	//Manager for the process phase of the frame pipeline
	class ProcessPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static ProcessPhase* GetInstance();

		//Run this phase asynchronously
		//Requires some world state to be committed
		std::future<void> Run() {
			return std::async(std::launch::async, [this](){
				this->Run();
			});
		}
	private:
		//Singleton members
		static ProcessPhase* instance;
		static bool instanceExists;

		void _Run();

		ProcessPhase() {}
	};
}
