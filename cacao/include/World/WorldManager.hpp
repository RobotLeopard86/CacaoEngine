#pragma once

#include <map>

#include "World.hpp"
#include "Utilities/Flushable.hpp"

namespace Cacao {
	//Singleton for managing global world state
	class WorldManager {
	public:
		//Get the instance or create one if it doesn't exist.
		static WorldManager* GetInstance();

		//Create an empty world
		void CreateWorld(std::string name);

		//Remove a world from the manager
		//This will also delete the world instance itself
		void RemoveWorld(std::string name);

		//Set the active world
		void SetActiveWorld(std::string name);

		//Get the ID of the active world
		std::string GetActiveWorld();

		//Access a world
		World& AccessWorld(std::string name);

	private:
		//Singleton members
		static WorldManager* instance;
		static bool instanceExists;

		//Map of world names to worlds
		std::map<std::string, World> worlds;

		//Active world ID
		std::string activeWorld;

		WorldManager() {}
	};
}
