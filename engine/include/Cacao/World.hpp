#pragma once

#include "Cubemap.hpp"
#include "DllHelper.hpp"
#include "Camera.hpp"
#include "Actor.hpp"
#include "Resource.hpp"

#include "libcacaoformats.hpp"

#include <memory>
#include <optional>

namespace Cacao {
	/**
	 * @brief A collection of entities and a camera comprising an area of gameplay
	 *
	 * @warning If there are still outstanding references to contained Actors when the destructor is called, it may not be able to prevent memory leaks and the Actor will probably be in a broken state.
	 */
	class CACAO_API World final : public Resource {
	  public:
		/**
		 * @brief Create a new blank world
		 *
		 * @param addr The resource address to associate with the world
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<World> Create(const std::string& addr) {
			return std::shared_ptr<World>(new World(addr));
		}

		/**
		 * @brief Create a new world using data
		 *
		 * @param addr The resource address to associate with the world
		 * @param world The world information for setup
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<World> Create(const std::string& addr, const libcacaoformats::World& world);

		std::shared_ptr<Camera> cam;///<World camera that will be used to render everything else

		std::shared_ptr<Cubemap> skyboxTex;///<Cube texture to use as the skybox

		/**
		 * @brief Make an Actor a toplevel (no parent, at root of world tree)
		 *
		 * @param actor The actor to make toplevel
		 *
		 * @throws BadValueException If the actor does not belong to this world
		 */
		void MakeToplevel(ActorRef actor);

		/**
		 * @brief Get a list of actors that are toplevel (have no parent)
		 */
		std::vector<ActorRef> GetToplevelActors() const;

		/**
		 * @brief Create a new Actor in the world
		 *
		 * @param name The name for this new actor
		 * @param parent The parent for this new actor (leave as a null ref to make toplevel)
		 *
		 * @throws NonexistentValueException If the ref references a nonexistent actor or is out-of-date
		 */
		ActorRef CreateActor(const std::string& name, ActorRef parent = {});

		/**
		 * @brief Find an Actor according to some arbitrary condition
		 *
		 * @param predicate The predicate to check each actor against
		 *
		 * @return An optional that contains the actor if it was found
		 */
		template<typename P>
			requires std::is_invocable_r_v<bool, P, ActorRef>
		std::optional<ActorRef> FindActor(P predicate) const {
			//Invoke internal search function with wrapped predicate
			return _FindActor([&predicate](ActorRef ref) { return predicate(ref); });
		}

		~World();

		///@cond
		struct Impl;
		///@endcond

	  private:
		World(const std::string& addr);
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		//Internal function for actually running a actor search
		std::optional<ActorRef> _FindActor(std::function<bool(ActorRef)> predicate) const;
	};
}