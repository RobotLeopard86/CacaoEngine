#pragma once

#include "DllHelper.hpp"
#include "Camera.hpp"
#include "Entity.hpp"

namespace Cacao {
	/**
	 * @brief A collection of entities and a camera comprising an area of gameplay
	 */
	class CACAO_API World {
	  public:
		std::shared_ptr<Camera> cam;///<World camera that will be used to render everything else

		/**
		 * @brief Set an entity's parent to the root entity (adding it to the world if it wasn't already)
		 *
		 * @param entity The entity to reparent
		 */
		void ReparentToRoot(std::shared_ptr<Entity> entity);

		/**
		 * @brief Get all entities that are direct children of the root entity
		 */
		std::vector<std::shared_ptr<Entity>> GetRootChildren();

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
			return entitySearchRunner(root->GetAllChildren(), predicate);
		}

	  private:
		World();

		std::shared_ptr<Entity> root;

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
				std::optional<std::shared_ptr<Entity>> found = entitySearchRunner(child->GetAllChildren(), predicate);
				if(found.has_value()) {
					return found;
				}
			}
			//Nothing was found, return null option
			return std::nullopt;
		}

		friend class WorldManager;
	};
}