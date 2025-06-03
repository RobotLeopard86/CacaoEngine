#pragma once

#include "DllHelper.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Singleton for handling the loading of resources from a game bundle
	 */
	class CACAO_API ResourceManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static ResourceManager& Get();

		///@cond
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;
		///@endcond
	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		ResourceManager();
		~ResourceManager();
	};
}