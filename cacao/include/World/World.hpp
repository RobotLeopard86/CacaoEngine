#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"
#include "Utilities/Tree.hpp"
#include "Graphics/Cameras/Camera.hpp"

#include <optional>
#include <algorithm>

namespace Cacao {
	//Represents a world
	class World {
	public:
		//Entities at the top level of the world
		Tree<Entity> worldTree;

		//Optional skybox
		std::optional<Skybox*> skybox;

		//Main camera
		//Will NOT be freed on object destruction
		Camera* cam;

		//Find an entity in this world by its UUID
		//Returns an optional because an entity may or may not be found
		std::optional<std::reference_wrapper<Entity>> FindEntityByUUID(UUIDv4::UUID uuid) {
			//Create a function to check if the UUID matches
			auto checkUUID = [uuid](Entity& e) {
				return uuid == e.uuid;
			};

			//Search for the object
			return entitySearchRunner(worldTree.children, checkUUID);
		}

		World(Camera* camera) {
			cam = camera;
		}
	private:
		//Recursive function for actually running a entity search
		template<typename P>
		std::optional<std::reference_wrapper<Entity>> entitySearchRunner(std::vector<TreeItem<Entity>>& target, P predicate){
			//Iterate through all children
			for(auto& child : target) {
				//Does this child pass the predicate?
				if(predicate(child.val())) {
					return std::optional<std::reference_wrapper<Entity>>(child.val());
				}
				//Search through children
				std::optional<std::reference_wrapper<Entity>> found = entitySearchRunner(child.children, predicate);
				if(found.has_value()) {
					return found;
				}
			}
			//Nothing was found, return null option
			return std::nullopt;
		}
	};
}