#pragma once

namespace Citrus {
	//Manager for the process phase of the frame pipeline
	class ProcessPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static ProcessPhase* GetInstance();

		//Run this phase
		//Requires some world state to be committed
		void Run();
	private:
		//Singleton members
		static ProcessPhase* instance;
		static bool instanceExists;

		ProcessPhase() {}
	}
}
