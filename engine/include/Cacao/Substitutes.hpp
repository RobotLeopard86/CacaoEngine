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

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<2, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	using vec_t = glm::vec<2, U>;
	U x;
	U y;

	//Converters
	SerializedSubstitute(const vec_t& vec) {
		x = vec.x;
		y = vec.y;
	}
	ASTRA_SUBSTITUTE_SERIALIZE(vec_t) {
		x = in->x;
		y = in->y;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(vec_t) {
		*out = vec_t {x, y};
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<3, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	using vec_t = glm::vec<3, U>;
	U x;
	U y;
	U z;

	//Converters
	SerializedSubstitute(const vec_t& vec) {
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}
	ASTRA_SUBSTITUTE_SERIALIZE(vec_t) {
		x = in->x;
		y = in->y;
		z = in->z;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(vec_t) {
		*out = vec_t {x, y, z};
	}

	SerializedSubstitute() = default;
};

template<typename U>
struct CACAO_API ASTRA_REFLECT astra::SerializedSubstitute<glm::vec<4, U>> : public AstraReflectBase {
	//Astra setup
	ASTRASETUP(SerializedSubstitute)
	virtual ~SerializedSubstitute() {}

	//Unpacked data representation
	using vec_t = glm::vec<4, U>;
	U x;
	U y;
	U z;
	U w;

	//Converters
	SerializedSubstitute(const vec_t& vec) {
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = vec.w;
	}
	ASTRA_SUBSTITUTE_SERIALIZE(vec_t) {
		x = in->x;
		y = in->y;
		z = in->z;
		w = in->w;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(vec_t) {
		*out = vec_t {x, y, z, w};
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
	SerializedSubstitute(const glm::qua<U>& quat) {
		x = quat.x;
		y = quat.y;
		z = quat.z;
		w = quat.w;
	}
	ASTRA_SUBSTITUTE_SERIALIZE(glm::qua<U>) {
		x = in->x;
		y = in->y;
		z = in->z;
		w = in->w;
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(glm::qua<U>) {
		*out = glm::qua<U> {x, y, z, w};
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
	using mat_t = glm::vec<4, U>;
	std::array<std::array<U, H>, W> data;

	//Converters
	SerializedSubstitute(const mat_t& mat) {
		for(uint8_t x = 0; x < W; ++x) {
			for(uint8_t y = 0; y < H; ++y) {
				data[x][y] = mat[x][y];
			}
		}
	}
	ASTRA_SUBSTITUTE_SERIALIZE(mat_t) {
		for(uint8_t x = 0; x < W; ++x) {
			for(uint8_t y = 0; y < H; ++y) {
				data[x][y] = (*in)[x][y];
			}
		}
	}
	ASTRA_SUBSTITUTE_DESERIALIZE(mat_t) {
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

	//Asset addresses of mesh and material
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
	SerializedSubstitute(const AudioPlayer& ap) {
		sound = ap.GetSound()->GetAddress();
		autoplay = ap.autoplay;
		loop = ap.GetLooping();
		gain = ap.GetGain();
		pitchMultiplier = ap.GetPitchMultiplier();
		playbackPosition = ap.GetPosition();
	}
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
	SerializedSubstitute(const Transform& transform) {
		{
			glm::vec3 p = transform.GetPosition();
			position.serialize(&p);
		}
		{
			glm::quat r = transform.GetRotation();
			rotation.serialize(&r);
		}
		{
			glm::vec3 s = transform.GetScale();
			scale.serialize(&s);
		}
	}
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