#include "World/WorldManager.hpp"

#include "Core/Log.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	//Required static variable initialization
	WorldManager* WorldManager::instance = nullptr;
	bool WorldManager::instanceExists = false;

	//Singleton accessor
	WorldManager* WorldManager::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new WorldManager();
			instanceExists = true;
		}

		return instance;
	}

	void WorldManager::RemoveWorld(std::string name) {
		CheckException(worlds.contains(name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't remove world because one with the specified name doesn't exist!")
		worlds.erase(name);
	}

	void WorldManager::SetActiveWorld(std::string name) {
		CheckException(worlds.contains(name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't set active world to a nonexistent one!")
		activeWorld = name;
	}

	std::string WorldManager::GetActiveWorldID() {
		return activeWorld;
	}

	World& WorldManager::GetWorld(std::string name) {
		CheckException(worlds.contains(name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't get access to a nonexistent world!")

		return worlds.at(name);
	}

	World& WorldManager::GetActiveWorld() {
		return GetWorld(GetActiveWorldID());
	}
}