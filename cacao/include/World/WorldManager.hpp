#pragma once

#include <map>

#include "World.hpp"
#include "Utilities/Flushable.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	/**
	 * @brief Interface for interacting with various worlds
	 */
	class WorldManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static WorldManager* GetInstance();

		/**
		 * @brief Create a new world
		 *
		 * @note The camera type template argument must subclass Camera
		 *
		 * @param name The name of the new world
		 *
		 * @throws Exception If there is already a world with the given name
		 */
		template<typename T>
		void CreateWorld(std::string name) {
			static_assert(std::is_base_of<Camera, T>(), "You must create a world with a camera type extending Camera!");
			CheckException(!worlds.contains(name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "A world with the provided name already exists!")

			worlds.insert_or_assign(name, World {new T()});
		}

		/**
		 * @brief Delete a world
		 *
		 * @param name The name of the world to delete
		 *
		 * @throws Exception If there is no world with the given name
		 */
		void RemoveWorld(std::string name);

		/**
		 * @brief Set the active world
		 *
		 * @param name The name of the world to set as the active world
		 *
		 * @throws Exception If there is no world with the given name
		 */
		void SetActiveWorld(std::string name);

		/**
		 * @brief Get the name of the active world
		 *
		 * @return The active world name or an empty string if there is no active world
		 */
		std::string GetActiveWorldID();

		/**
		 * @brief Get the active world
		 *
		 * @return A mutable reference to the active world
		 *
		 * @throws Exception If there is no active world
		 */
		World& GetActiveWorld();

		/**
		 * @brief Get the world with the given name
		 *
		 * @return The world with the given name
		 *
		 * @throws Exception If there is no world with the given name
		 */
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
