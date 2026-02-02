#pragma once

#include "Cacao/World.hpp"

#include <set>
#include <vector>

namespace Cacao {
	struct World::Impl {
		struct ActorSlot {
			uint64_t generation = 1;
			std::unique_ptr<Actor> actor;
		};

		std::vector<ActorSlot> slotTable;
		std::set<uint64_t> freeList;
	};
}