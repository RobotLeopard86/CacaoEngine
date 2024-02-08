#include "Pipeline/LogicPhase.hpp"

#include "World/WorldManager.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Utilities/Input.hpp"

namespace Citrus {
	//Required static variable initialization
	LogicPhase* LogicPhase::instance = nullptr;
	bool LogicPhase::instanceExists = false;

	//Singleton accessor
	LogicPhase* LogicPhase::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new LogicPhase();
			instanceExists = true;
		}

		return instance;
	}

	bool LogicPhase::IsScriptRegistered(Script& script){
		auto it = std::find_if(scripts.cbegin(), scripts.cend(), [script](std::reference_wrapper<Script> value){
			return static_cast<Script>(script) == value.get();
		});
		return it != scripts.cend();
	}

	void LogicPhase::RegisterScript(Script& script){
		if(IsScriptRegistered(script)){
			Logging::EngineLog("Can't register script with the logic phase when that script is already registered!", LogLevel::Error);
			return;
		}

		scripts.push_back(script);
	}

	void LogicPhase::UnregisterScript(Script& script){
		if(!IsScriptRegistered(script)){
			Logging::EngineLog("Can't unregister script from the logic phase when that script isn't registered!", LogLevel::Error);
			return;
		}

		scripts.erase(std::find_if(scripts.cbegin(), scripts.cend(), [script](std::reference_wrapper<Script> value){
			return static_cast<Script>(script) == value.get();
		}));
	}

	void LogicPhase::_Run() {
		//Freeze input state for this frame
		Input::GetInstance()->FreezeFrameInputState();

		//Run all scripts
		for(Script& s : scripts){
			if(s.IsActive()) s.OnTick();
		}

		//Commit world state
		WorldManager::GetInstance()->Commit();
	}
}
