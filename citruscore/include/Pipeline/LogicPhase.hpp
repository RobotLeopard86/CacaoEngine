#pragma once

#include "Scripts/Script.hpp"

#include <vector>

namespace Citrus {
	//Manager for the logic phase of the frame pipeline
	class LogicPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static LogicPhase* GetInstance();

		//Run this phase
		void Run();
		
		//Register a script for use during the logic phase
		//Script will not be run if its active value is false
		void RegisterScript(Script& script);

		//Unregister a script from logic phase use
		void UnregisterScript(Script& script);

		//Check if a script is to be used
		void IsScriptRegistered(Script& script);
	private:
		//Singleton members
		static LogicPhase* instance;
		static bool instanceExists;

		//Registered scripts
		std::vector<Script&> scripts;

		LogicPhase() {}
	}
}
