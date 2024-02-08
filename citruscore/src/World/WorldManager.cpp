#include "World/WorldManager.hpp"

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

namespace Citrus {
	//Required static variable initialization
	WorldManager* WorldManager::instance = nullptr;
	bool WorldManager::instanceExists = false;

	//Singleton accessor
	WorldManager* WorldManager::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new WorldManager();
			instanceExists = true;
		}

		return instance;
	}

	void WorldManager::CreateWorld(std::string name){
		if(worlds.contains(name)) {
			Logging::EngineLog("Not adding world because one with the specified name already exists!", LogLevel::Warn);
			return;
		}

		worlds.insert_or_assign(name, World{});
	}

	void WorldManager::RemoveWorld(std::string name){
		if(!worlds.contains(name)) {
			Logging::EngineLog("Can't remove world because one with the specified name doesn't exist!", LogLevel::Error);
			return;
		}

		worlds.erase(name);
	}

	void WorldManager::SetActiveWorld(std::string name){
		if(!worlds.contains(name)) {
			Logging::EngineLog("Can't set active world to a world that doesn't exist!", LogLevel::Error);
			return;
		}

		activeWorld = name;
	}

	std::string WorldManager::GetActiveWorld(){
		return activeWorld;
	}

	World& WorldManager::AccessWorld(std::string name){
		Asserts::EngineAssert(worlds.contains(name), "Can't access a world that doesn't exist!");

		return worlds[name];
	}

	void WorldManager::Commit(){
		Asserts::EngineAssert(worlds.contains(activeWorld), "Can't commit world state for a world that doesn't exist!");

		commitedWorld = World(worlds[activeWorld]);
		commitedWorldExists = true;
	}
}
