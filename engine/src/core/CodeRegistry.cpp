#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Actor.hpp"
#include "SingletonGet.hpp"

#include <memory>
#include <typeindex>

#define INSTANTIATED(tp) std::pair<tp*, std::type_index>

namespace Cacao {
	//In case it wasn't obvious, everything in this file is TODO
	struct CodeRegistry::Impl {
	};

	CodeRegistry::CodeRegistry() {}
	CodeRegistry::~CodeRegistry() {}

	CACAOST_GET(CodeRegistry)

	template<>
	void CodeRegistry::RegisterFactory<Component>(const std::string& id, std::function<Component*()> factory, std::type_index type) {
		//TODO
	}

	template<>
	bool CodeRegistry::HasFactory<Component>(const std::string& id) {
		//TODO
		return true;
	}

	template<>
	INSTANTIATED(Component)
	CodeRegistry::Instantiate<Component>(const std::string& id) {
		//TODO
		return std::make_pair<Component*, std::type_index>(nullptr, std::type_index(typeid(Component)));
	}
}