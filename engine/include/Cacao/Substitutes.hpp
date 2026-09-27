#pragma once

#include "Cacao/Transform.hpp"
#include "Exceptions.hpp"
#include "MeshRenderer.hpp"
#include "Resource.hpp"
#include "ResourceManager.hpp"
#include "AudioPlayer.hpp"
#include "DllHelper.hpp"
#include "Actor.hpp"
#include "World.hpp"
#include "ResourceManager.hpp"

#include "astra/serialized_substitute.hpp"
#include "astra/setup.hpp"

#include "crossguid/guid.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <cstdint>

using namespace Cacao;

template<>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<xg::Guid> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID
	std::string data;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(xg::Guid) {
		data = in->str();
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(xg::Guid) {
		*out = xg::Guid(data);
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<2, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	U x;
	U y;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(glm::vec<2, U>) {
		x = in->x;
		y = in->y;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::vec<2, U>) {
		*out = glm::vec<2, U> {x, y};
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<3, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	U x;
	U y;
	U z;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(glm::vec<3, U>) {
		x = in->x;
		y = in->y;
		z = in->z;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::vec<3, U>) {
		*out = glm::vec<3, U> {x, y, z};
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<4, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	U x;
	U y;
	U z;
	U w;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(glm::vec<4, U>) {
		x = in->x;
		y = in->y;
		z = in->z;
		w = in->w;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::vec<4, U>) {
		*out = glm::vec<4, U> {x, y, z, w};
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::qua<U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	U w;
	U x;
	U y;
	U z;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(glm::qua<U>) {
		x = in->x;
		y = in->y;
		z = in->z;
		w = in->w;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::qua<U>) {
		*out = glm::qua<U> {w, x, y, z};
	}

	SerializedSubstitute() = default;
};

template<uint8_t W, uint8_t H, typename U>
	requires(W >= 2 && W <= 4 && H >= 2 && H <= 4)
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::mat<W, H, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	std::array<std::array<U, H>, W> data;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(glm::mat<W, H, U>) {
		for(uint8_t x = 0; x < W; ++x) {
			for(uint8_t y = 0; y < H; ++y) {
				data[x][y] = (*in)[x][y];
			}
		}
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::mat<W, H, U>) {
		for(uint8_t x = 0; x < W; ++x) {
			for(uint8_t y = 0; y < H; ++y) {
				(*out)[x][y] = data[x][y];
			}
		}
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

	//Asset addresses of mesh and material
	std::string mesh;
	std::string material;

	//Converters
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

template<>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<AudioPlayer> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Serialized string GUID of actor
	std::string sound;

	//Settings
	bool autoplay = false;
	bool loop = false;
	float gain = 1.0f;
	float pitchMultiplier = 1.0f;
	float playbackPosition = 0.0f;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(AudioPlayer) {
		sound = in->GetSound()->GetAddress();
		autoplay = in->autoplay;
		loop = in->GetLooping();
		gain = in->GetGain();
		pitchMultiplier = in->GetPitchMultiplier();
		playbackPosition = in->GetPosition();
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(AudioPlayer) {
		out->autoplay = autoplay;
		out->SetSound(*ResourceManager::Get().Load<Sound>(sound));
		out->SetLooping(loop);
		out->SetGain(gain);
		out->SetPitchMultiplier(pitchMultiplier);
		out->SetPosition(playbackPosition);
	}

	SerializedSubstitute() = default;
};

template<>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<Transform> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Asset addresses of mesh and material
	astra::SerializedSubstitute<glm::vec3> position;
	astra::SerializedSubstitute<glm::quat> rotation;
	astra::SerializedSubstitute<glm::vec3> scale;

	//Converters
	ASTRA_SUBSTITUTE_SERIALIZE(Transform) {
		{
			glm::vec3 p = in->GetPosition();
			position.serialize(&p);
		}
		{
			glm::quat r = in->GetRotation();
			rotation.serialize(&r);
		}
		{
			glm::vec3 s = in->GetScale();
			scale.serialize(&s);
		}
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(Transform) {
		{
			glm::vec3 p;
			position.deserialize(&p);
			out->SetPosition(p);
		}
		{
			glm::quat r;
			rotation.deserialize(&r);
			out->SetRotation(r);
		}
		{
			glm::vec3 s;
			scale.deserialize(&s);
			out->SetScale(s);
		}
	}

	SerializedSubstitute() = default;
};