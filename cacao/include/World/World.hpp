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
		//Optional skybox
		std::optional<Skybox*> skybox;

		//Main camera
		//Will NOT be freed on object destruction
		Camera* cam;

		//Root entity of this world
		std::shared_ptr<Entity> rootEntity;

		//Find an entity in this world by its GUID
		//Returns an optional because an entity may or may not be found
		std::optional<std::shared_ptr<Entity>> FindEntityByGUID(xg::Guid guid) {
			//Create a function to check if the GUID matches
			auto checkGUID = [guid](std::shared_ptr<Entity> e) {
				return guid == e->guid;
			};

			//Search for the object
			return entitySearchRunner(rootEntity->GetChildrenAsList(), checkGUID);
		}

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