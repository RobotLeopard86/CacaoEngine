#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Exceptions.hpp"
#include "SingletonGet.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>

#define INSTANTIATED(tp) std::pair<tp*, std::type_index>

namespace Cacao {
	struct CodeRegistry::Impl {
		struct Factory {
			std::function<Component*()> factory;
			std::type_index type;
		};
		std::unordered_map<std::type_index, std::unordered_map<std::string, std::unique_ptr<Factory>>> factories;
	};

	CodeRegistry::CodeRegistry() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	CodeRegistry::~CodeRegistry() {}

	CACAOST_GET(CodeRegistry)

	void CodeRegistry::ClearAllFactories() {
		impl->factories.clear();
	}

	template<>
	bool CodeRegistry::HasFactory<Component>(const std::string& id) {
		return impl->factories[typeid(Component)].contains(id);
	}

	template<>
	void CodeRegistry::RegisterFactory<Component>(const std::string& id, std::function<Component*()> factory, std::type_index type) {
		Check<ExistingValueException>(!HasFactory<Component>(id), "Cannot register a component factory using an ID belonging to another existing factory!");
		impl->factories[typeid(Component)][id] = std::unique_ptr<Impl::Factory>(new Impl::Factory {.factory = factory, .type = type});
	}

	template<>
	INSTANTIATED(Component)
	CodeRegistry::Instantiate<Component>(const std::string& id) {
		Check<NonexistentValueException>(HasFactory<Component>(id), "Cannot instantiate a component from an unregistered factory!");
		std::unique_ptr<Impl::Factory>& factory = impl->factories[typeid(Component)][id];
		return std::make_pair<Component*, std::type_index>(factory->factory(), std::type_index(factory->type));
	}
}