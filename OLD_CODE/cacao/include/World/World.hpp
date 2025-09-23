#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Cameras/Camera.hpp"

#include <optional>
#include <algorithm>

namespace Cacao {
	/**
	 * @brief A gameplay space
	 */
	class World {
	  public:
		AssetHandle<Skybox> skybox;///<Skybox (set to a null asset handle to have no skybox)

		Camera* cam;///<Main camera (will not be freed on world destruction)

		std::shared_ptr<Entity> rootEntity;///<Root entity (set this as an Entity's parent to add it to the world)

		/**
		 * @brief Find an Entity by some arbitrary condition
		 *
		 * @param predicate The predicate to check each entity against
		 *
		 * @return An optional that contains the entity if it was found
		 */
		template<typename P>
		std::optional<std::shared_ptr<Entity>> FindEntity(P predicate) {
			//Search for the object
			return entitySearchRunner(rootEntity->GetChildrenAsList(), predicate);
		}

		/**
		 * @brief Create a world
		 *
		 * @param camera The main camera
		 *
		 * @note Prefer to use WorldManager::CreateWorld to be able to use the created world
		 */
		World(Camera* camera) {
			cam = camera;
			rootEntity = std::make_shared<Entity>("__WORLDROOT__");
			rootEntity->GetLocalTransform().SetPosition({0, 0, 0});
			rootEntity->GetLocalTransform().SetRotation({0, 0, 0});
			rootEntity->GetLocalTransform().SetScale({1, 1, 1});
		}

	  private:
		//Recursive function for actually running a entity search
		template<typename P>
		std::optional<std::shared_ptr<Entity>> entitySearchRunner(std::vector<std::shared_ptr<Entity>> target, P predicate) {
			//Iterate through all children
			for(auto child : target) {
				//Does this child pass the predicate?
				if(predicate(child)) {
					return std::optional<std::shared_ptr<Entity>>(child);
				}
				//Search through children
				std::optional<std::shared_ptr<Entity>> found = entitySearchRunner(child->GetChildrenAsList(), predicate);
				if(found.has_value()) {
					return found;
				}
			}
			//Nothing was found, return null option
			return std::nullopt;
		}
	};
}