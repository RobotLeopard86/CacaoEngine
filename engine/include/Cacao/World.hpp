#pragma once

#include "DllHelper.hpp"
#include "Camera.hpp"
#include "Actor.hpp"
#include "Resource.hpp"

namespace Cacao {
	/**
	 * @brief A collection of entities and a camera comprising an area of gameplay
	 *
	 * @warning If there are still outstanding references to contained Actors when the destructor is called, it may not be able to prevent memory leaks and the Actor will probably be in a broken state.
	 */
	class CACAO_API World : public Resource {
	  public:
		std::shared_ptr<Camera> cam;///<World camera that will be used to render everything else

		/**
		 * @brief Set an actor's parent to the root actor (adding it to the world if it wasn't already)
		 *
		 * @param actor The actor to reparent
		 */
		void ReparentToRoot(std::shared_ptr<Actor> actor);

		/**
		 * @brief Get all entities that are direct children of the root actor
		 */
		std::vector<std::shared_ptr<Actor>> GetRootChildren();

		/**
		 * @brief Find an Actor by some arbitrary condition
		 *
		 * @param predicate The predicate to check each actor against
		 *
		 * @return An optional that contains the actor if it was found
		 */
		template<typename P>
		std::optional<std::shared_ptr<Actor>> FindActor(P predicate) {
			//Search for the object
			return actorSearchRunner(root->GetAllChildren(), predicate);
		}

	  private:
		World(const std::string& addr);
		~World();

		std::shared_ptr<Actor> root;

		//Recursive function for actually running a actor search
		template<typename P>
		std::optional<std::shared_ptr<Actor>> actorSearchRunner(std::vector<std::shared_ptr<Actor>> target, P predicate) {
			//Iterate through all children
			for(auto child : target) {
				//Does this child pass the predicate?
				if(predicate(child)) {
					return std::optional<std::shared_ptr<Actor>>(child);
				}
				//Search through children
				std::optional<std::shared_ptr<Actor>> found = actorSearchRunner(child->GetAllChildren(), predicate);
				if(found.has_value()) {
					return found;
				}
			}
			//Nothing was found, return null option
			return std::nullopt;
		}

		friend class ResourceManager;
	};
}