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

		//Create an empty world with a camera
		//Defined in the header file to allow usage by the client
		template<typename T>
		void CreateWorld(std::string name){
			static_assert(std::is_base_of<Camera, T>(), "You must create a world with a type extending Camera!");
			if(worlds.contains(name)) {
				Logging::EngineLog("Not adding world because one with the specified name already exists!", LogLevel::Warn);
				return;
			}

			worlds.insert_or_assign(name, World{new T()});
		}

		//Remove a world from the manager
		//This will also delete the world instance itself
		void RemoveWorld(std::string name);

		//Set the active world
		void SetActiveWorld(std::string name);

		//Get the ID of the active world
		std::string GetActiveWorldID();

		//Get a reference to the active world
		World& GetActiveWorld();

		//Get a reference to a world
		World& GetWorld(std::string name);
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
