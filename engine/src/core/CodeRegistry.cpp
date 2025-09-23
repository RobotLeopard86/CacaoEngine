#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Component.hpp"
#include "SingletonGet.hpp"

#include <memory>
#include <typeindex>

#define INSTANTIATED(tp) std::pair<std::shared_ptr<tp>, std::type_index>

namespace Cacao {
	//In case it wasn't obvious, this is TODO
	struct CodeRegistry::Impl {
	};

	CodeRegistry::CodeRegistry() {}
	CodeRegistry::~CodeRegistry() {}

	CACAOST_GET(CodeRegistry)

	template<>
	void CodeRegistry::RegisterFactory<Component>(const std::string& id, std::function<Component*()> factory, std::type_index type) {}

	template<>
	void CodeRegistry::RegisterFactory<Script>(const std::string& id, std::function<Script*()> factory, std::type_index type) {}

	template<>
	INSTANTIATED(Component)
	CodeRegistry::Instantiate<Component>(const std::string& id) {
		return std::make_pair<std::shared_ptr<Component>, std::type_index>(std::shared_ptr<Component>(), std::type_index(typeid(Component)));
	}

	template<>
	INSTANTIATED(Script)
	CodeRegistry::Instantiate<Script>(const std::string& id) {
		return std::make_pair<std::shared_ptr<Script>, std::type_index>(std::shared_ptr<Script>(), std::type_index(typeid(Component)));
	}
}