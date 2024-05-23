#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Cameras/Camera.hpp"

#include <optional>
#include <algorithm>

namespace Cacao {
	//Represents a world
	class World {
	  public:
		//Entities at the top level of the world
		std::vector<std::shared_ptr<Entity>> topLevelEntities;

		//Optional skybox
		std::optional<Skybox*> skybox;

		//Main camera
		//Will NOT be freed on object destruction
		Camera* cam;

		//Find an entity in this world by its UUID
		//Returns an optional because an entity may or may not be found
		std::optional<std::shared_ptr<Entity>> FindEntityByUUID(UUIDv4::UUID uuid) {
			//Create a function to check if the UUID matches
			auto checkUUID = [uuid](std::shared_ptr<Entity> e) {
				return uuid == e->uuid;
			};

			//Search for the object
			return entitySearchRunner(topLevelEntities, checkUUID);
		}

		World(Camera* camera) {
			cam = camera;
		}

	  private:
		//Recursive function for actually running a entity search
		template<typename P>
		std::optional<std::shared_ptr<Entity>> entitySearchRunner(std::vector<std::shared_ptr<Entity>>& target, P predicate) {
			//Iterate through all children
			for(auto child : target) {
				//Does this child pass the predicate?
				if(predicate(child)) {
					return std::optional<std::shared_ptr<Entity>>(child);
				}
				//Search through children
				std::optional<std::shared_ptr<Entity>> found = entitySearchRunner(child->children, predicate);
				if(found.has_value()) {
					return found;
				}
			}
			//Nothing was found, return null option
			return std::nullopt;
		}
	};
}