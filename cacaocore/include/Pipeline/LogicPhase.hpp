#pragma once

#include "Scripts/Script.hpp"

#include <vector>
#include <future>

namespace Cacao {
	//Manager for the logic phase of the frame pipeline
	class LogicPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static LogicPhase* GetInstance();

		//Run this phase asynchronously
		std::future<void> Run() {
			return std::async(std::launch::async, [this](){
				this->Run();
			});
		}
		
		//Register a script for use during the logic phase
		//Script will not be run if its active value is false
		void RegisterScript(Script& script);

		//Unregister a script from logic phase use
		void UnregisterScript(Script& script);

		//Check if a script is to be used
		bool IsScriptRegistered(Script& script);
	private:
		//Singleton members
		static LogicPhase* instance;
		static bool instanceExists;

		//Registered scripts
		std::vector<std::reference_wrapper<Script>> scripts;

		//Actual run code
		void _Run();

		LogicPhase() {}
	};
}
