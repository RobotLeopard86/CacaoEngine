#pragma once

#include "DllHelper.hpp"
#include "World.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Active world management singleton
	 */
	class CACAO_API WorldManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static WorldManager& Get();

		///@cond
		WorldManager(const WorldManager&) = delete;
		WorldManager(WorldManager&&) = delete;
		WorldManager& operator=(const WorldManager&) = delete;
		WorldManager& operator=(WorldManager&&) = delete;
		///@endcond

		/**
		 * @brief Set the active world
		 *
		 * @param addr The resource address of the world to activate
		 * @param noload Set this to true to not load the world if it isn't currently loaded
		 *
		 * @throws BadValueException If the provided address is malformed
		 * @throws NonexistentValueException If the provided address does not reference a world
		 * @throws NonexistentValueException If noload is set and the world is not currently loaded
		 */
		void SetActiveWorld(const std::string& addr, bool noload = false);

		/**
		 * @brief Access the active World object
		 *
		 * @return A handle to the active world, or an empty pointer if no world is active
		 */
		std::shared_ptr<World> GetActiveWorld();

		/**
		 * @brief Check the active world's resource address
		 *
		 * @return The resource address of the active world, or an empty string if no world is active
		 */
		std::string GetActiveWorldAddr();

		///@cond
		struct Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		WorldManager();
		~WorldManager();
	};
}