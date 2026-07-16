#pragma once

#include "Cacao/Exceptions.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/ResourceManager.hpp"
#include "DllHelper.hpp"
#include "Actor.hpp"
#include "World.hpp"
#include "ResourceManager.hpp"

#include "astra/serialized_substitute.hpp"
#include "astra/setup.hpp"

#include "crossguid/guid.hpp"
#include "glm/glm.hpp"

using namespace Cacao;

template<>
struct ASTRA_REFLECT astra::SerializedSubstitute<xg::Guid> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID
	std::string data;

	//Converters
	SerializedSubstitute(const xg::Guid& guid) {
		data = guid.str();
	}
	xg::Guid deserialize() {
		return xg::Guid(data);
	}

	SerializedSubstitute() = default;
};

template<>
struct ASTRA_REFLECT astra::SerializedSubstitute<ActorRef> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID of actor
	std::string guid;
	std::string worldAddr;

	//Converters
	SerializedSubstitute(const ActorRef& ref) {
		guid = ref->guid;
		worldAddr = ref.GetWorld()->GetAddress();
	}
	ActorRef deserialize() {
		std::vector<ActorRef> results = (*ResourceManager::Get().Load<World>(worldAddr))->FindActors([this](ActorRef r) {
			return r->guid == xg::Guid(guid);
		});
		Check<BadValueException>(results.size() == 1, "Bad actor ref deserialization!");
		return results[0];
	}

	SerializedSubstitute() = default;
};