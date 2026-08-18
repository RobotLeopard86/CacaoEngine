#pragma once

#include "Cacao/Exceptions.hpp"
#include "Cacao/MeshRenderer.hpp"
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
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<xg::Guid> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID
	std::string data;

	//Converters
	SerializedSubstitute(const xg::Guid& guid) {
		data = guid.str();
	}
	ASTRA_SUBSTITUTE_SERIALIZE(xg::Guid) {
		data = in->str();
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(xg::Guid) {
		*out = xg::Guid(data);
	}

	SerializedSubstitute() = default;
};

template<>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<ActorRef> : public AstraReflectBase {
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
	ASTRA_SUBSTITUTE_SERIALIZE(ActorRef) {
		guid = (*in)->guid;
		worldAddr = in->GetWorld()->GetAddress();
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(ActorRef) {
		std::vector<ActorRef> results = (*ResourceManager::Get().Load<World>(worldAddr))->FindActors([this](ActorRef r) {
			return r->guid == xg::Guid(guid);
		});
		Check<BadValueException>(results.size() == 1, "Bad actor ref deserialization!");
		*out = results[0];
	}

	SerializedSubstitute() = default;
};

template<>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<MeshRenderer> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID of actor
	std::string mesh;
	std::string material;

	//Converters
	SerializedSubstitute(const MeshRenderer& mr) {
		mesh = mr.mesh->GetAddress();
		material = mr.material->GetAddress();
	}
	ASTRA_SUBSTITUTE_SERIALIZE(MeshRenderer) {
		mesh = in->mesh->GetAddress();
		material = in->material->GetAddress();
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(MeshRenderer) {
		exathread::Future<std::shared_ptr<Mesh>> meshFut = ResourceManager::Get().Load<Mesh>(mesh);
		exathread::Future<std::shared_ptr<Material>> matFut = ResourceManager::Get().Load<Material>(material);
		out->mesh = *meshFut;
		out->material = *matFut;
	}

	SerializedSubstitute() = default;
};
