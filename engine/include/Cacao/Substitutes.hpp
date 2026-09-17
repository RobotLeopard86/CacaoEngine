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

//This block is to force template instantiation
///@cond
#ifdef _ASTRAGENERATE
struct _ForceInstantiate {
	astra::SerializedSubstitute<xg::Guid> _0;
	astra::SerializedSubstitute<ActorRef> _1;
	astra::SerializedSubstitute<MeshRenderer> _2;
	astra::SerializedSubstitute<AudioPlayer> _3;
	astra::SerializedSubstitute<Transform> _4;
	astra::SerializedSubstitute<glm::bvec2> _5;
	astra::SerializedSubstitute<glm::i8vec2> _6;
	astra::SerializedSubstitute<glm::i16vec2> _7;
	astra::SerializedSubstitute<glm::i32vec2> _8;
	astra::SerializedSubstitute<glm::i64vec2> _9;
	astra::SerializedSubstitute<glm::u8vec2> _10;
	astra::SerializedSubstitute<glm::u16vec2> _11;
	astra::SerializedSubstitute<glm::u32vec2> _12;
	astra::SerializedSubstitute<glm::u64vec2> _13;
	astra::SerializedSubstitute<glm::dvec2> _14;
	astra::SerializedSubstitute<glm::vec2> _15;
	astra::SerializedSubstitute<glm::bvec3> _16;
	astra::SerializedSubstitute<glm::i8vec3> _17;
	astra::SerializedSubstitute<glm::i16vec3> _18;
	astra::SerializedSubstitute<glm::i32vec3> _19;
	astra::SerializedSubstitute<glm::i64vec3> _20;
	astra::SerializedSubstitute<glm::u8vec3> _21;
	astra::SerializedSubstitute<glm::u16vec3> _22;
	astra::SerializedSubstitute<glm::u32vec3> _23;
	astra::SerializedSubstitute<glm::u64vec3> _24;
	astra::SerializedSubstitute<glm::dvec3> _25;
	astra::SerializedSubstitute<glm::vec3> _26;
	astra::SerializedSubstitute<glm::bvec4> _27;
	astra::SerializedSubstitute<glm::i8vec4> _28;
	astra::SerializedSubstitute<glm::i16vec4> _29;
	astra::SerializedSubstitute<glm::i32vec4> _30;
	astra::SerializedSubstitute<glm::i64vec4> _31;
	astra::SerializedSubstitute<glm::u8vec4> _32;
	astra::SerializedSubstitute<glm::u16vec4> _33;
	astra::SerializedSubstitute<glm::u32vec4> _34;
	astra::SerializedSubstitute<glm::u64vec4> _35;
	astra::SerializedSubstitute<glm::dvec4> _36;
	astra::SerializedSubstitute<glm::vec4> _37;
	astra::SerializedSubstitute<glm::mat2x2> _38;
	astra::SerializedSubstitute<glm::dmat2x2> _39;
	astra::SerializedSubstitute<glm::i8mat2x2> _40;
	astra::SerializedSubstitute<glm::i16mat2x2> _41;
	astra::SerializedSubstitute<glm::i32mat2x2> _42;
	astra::SerializedSubstitute<glm::i64mat2x2> _43;
	astra::SerializedSubstitute<glm::u8mat2x2> _44;
	astra::SerializedSubstitute<glm::u16mat2x2> _45;
	astra::SerializedSubstitute<glm::u32mat2x2> _46;
	astra::SerializedSubstitute<glm::u64mat2x2> _47;
	astra::SerializedSubstitute<glm::mat2x3> _48;
	astra::SerializedSubstitute<glm::dmat2x3> _49;
	astra::SerializedSubstitute<glm::i8mat2x3> _50;
	astra::SerializedSubstitute<glm::i16mat2x3> _51;
	astra::SerializedSubstitute<glm::i32mat2x3> _52;
	astra::SerializedSubstitute<glm::i64mat2x3> _53;
	astra::SerializedSubstitute<glm::u8mat2x3> _54;
	astra::SerializedSubstitute<glm::u16mat2x3> _55;
	astra::SerializedSubstitute<glm::u32mat2x3> _56;
	astra::SerializedSubstitute<glm::u64mat2x3> _57;
	astra::SerializedSubstitute<glm::mat2x4> _58;
	astra::SerializedSubstitute<glm::dmat2x4> _59;
	astra::SerializedSubstitute<glm::i8mat2x4> _60;
	astra::SerializedSubstitute<glm::i16mat2x4> _61;
	astra::SerializedSubstitute<glm::i32mat2x4> _62;
	astra::SerializedSubstitute<glm::i64mat2x4> _63;
	astra::SerializedSubstitute<glm::u8mat2x4> _64;
	astra::SerializedSubstitute<glm::u16mat2x4> _65;
	astra::SerializedSubstitute<glm::u32mat2x4> _66;
	astra::SerializedSubstitute<glm::u64mat2x4> _67;
	astra::SerializedSubstitute<glm::mat3x2> _68;
	astra::SerializedSubstitute<glm::dmat3x2> _69;
	astra::SerializedSubstitute<glm::i8mat3x2> _70;
	astra::SerializedSubstitute<glm::i16mat3x2> _71;
	astra::SerializedSubstitute<glm::i32mat3x2> _72;
	astra::SerializedSubstitute<glm::i64mat3x2> _73;
	astra::SerializedSubstitute<glm::u8mat3x2> _74;
	astra::SerializedSubstitute<glm::u16mat3x2> _75;
	astra::SerializedSubstitute<glm::u32mat3x2> _76;
	astra::SerializedSubstitute<glm::u64mat3x2> _77;
	astra::SerializedSubstitute<glm::mat3x3> _78;
	astra::SerializedSubstitute<glm::dmat3x3> _79;
	astra::SerializedSubstitute<glm::i8mat3x3> _80;
	astra::SerializedSubstitute<glm::i16mat3x3> _81;
	astra::SerializedSubstitute<glm::i32mat3x3> _82;
	astra::SerializedSubstitute<glm::i64mat3x3> _83;
	astra::SerializedSubstitute<glm::u8mat3x3> _84;
	astra::SerializedSubstitute<glm::u16mat3x3> _85;
	astra::SerializedSubstitute<glm::u32mat3x3> _86;
	astra::SerializedSubstitute<glm::u64mat3x3> _87;
	astra::SerializedSubstitute<glm::mat3x4> _88;
	astra::SerializedSubstitute<glm::dmat3x4> _89;
	astra::SerializedSubstitute<glm::i8mat3x4> _90;
	astra::SerializedSubstitute<glm::i16mat3x4> _91;
	astra::SerializedSubstitute<glm::i32mat3x4> _92;
	astra::SerializedSubstitute<glm::i64mat3x4> _93;
	astra::SerializedSubstitute<glm::u8mat3x4> _94;
	astra::SerializedSubstitute<glm::u16mat3x4> _95;
	astra::SerializedSubstitute<glm::u32mat3x4> _96;
	astra::SerializedSubstitute<glm::u64mat3x4> _97;
	astra::SerializedSubstitute<glm::mat4x2> _98;
	astra::SerializedSubstitute<glm::dmat4x2> _99;
	astra::SerializedSubstitute<glm::i8mat4x2> _100;
	astra::SerializedSubstitute<glm::i16mat4x2> _101;
	astra::SerializedSubstitute<glm::i32mat4x2> _102;
	astra::SerializedSubstitute<glm::i64mat4x2> _103;
	astra::SerializedSubstitute<glm::u8mat4x2> _104;
	astra::SerializedSubstitute<glm::u16mat4x2> _105;
	astra::SerializedSubstitute<glm::u32mat4x2> _106;
	astra::SerializedSubstitute<glm::u64mat4x2> _107;
	astra::SerializedSubstitute<glm::mat4x3> _108;
	astra::SerializedSubstitute<glm::dmat4x3> _109;
	astra::SerializedSubstitute<glm::i8mat4x3> _110;
	astra::SerializedSubstitute<glm::i16mat4x3> _111;
	astra::SerializedSubstitute<glm::i32mat4x3> _112;
	astra::SerializedSubstitute<glm::i64mat4x3> _113;
	astra::SerializedSubstitute<glm::u8mat4x3> _114;
	astra::SerializedSubstitute<glm::u16mat4x3> _115;
	astra::SerializedSubstitute<glm::u32mat4x3> _116;
	astra::SerializedSubstitute<glm::u64mat4x3> _117;
	astra::SerializedSubstitute<glm::mat4x4> _118;
	astra::SerializedSubstitute<glm::dmat4x4> _119;
	astra::SerializedSubstitute<glm::i8mat4x4> _120;
	astra::SerializedSubstitute<glm::i16mat4x4> _121;
	astra::SerializedSubstitute<glm::i32mat4x4> _122;
	astra::SerializedSubstitute<glm::i64mat4x4> _123;
	astra::SerializedSubstitute<glm::u8mat4x4> _124;
	astra::SerializedSubstitute<glm::u16mat4x4> _125;
	astra::SerializedSubstitute<glm::u32mat4x4> _126;
	astra::SerializedSubstitute<glm::u64mat4x4> _127;
	astra::SerializedSubstitute<glm::quat> _128;
	astra::SerializedSubstitute<glm::dquat> _129;
};
#endif
///@endcond