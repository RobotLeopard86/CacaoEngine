#include "World/WorldManager.hpp"

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

namespace Cacao {
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

	std::string WorldManager::GetActiveWorldID(){
		return activeWorld;
	}

	World& WorldManager::GetWorld(std::string name){
		EngineAssert(worlds.contains(name), "Can't get access to a world that doesn't exist!");

		return worlds.at(name);
	}

	World& WorldManager::GetActiveWorld(){
		return GetWorld(GetActiveWorldID());
	}
}