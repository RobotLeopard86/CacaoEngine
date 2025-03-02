#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include "yaml-cpp/yaml.h"
#include "er/serialization/yaml.h"
#include "er/serialization/binary.h"

namespace libcacaoformats {
	void UnpackedEncoder::EncodeMaterial(const Material& mat, std::ostream& out) {
		//Create YAML emitter and write simple stuff
		YAML::Emitter yml;
		yml << YAML::Key << "shader" << YAML::Value << mat.shader;

		//Write out each value
		yml << YAML::Key << "data" << YAML::BeginSeq;
		for(const auto& item : mat.keys) {
			yml << YAML::BeginMap;
			yml << YAML::Key << "name" << YAML::Value << item.first;

			/*
			Yes there are no comments in all of this. I figured that it's simplistic enough, but here's a breakdown just in case:
			The number we use is based on the order in the ValueContainer template.
			For each type, we write a "base type" that represents what data is being stored
			Then we write size values with the "x" and "y" values.
			Lastly, we write the actual data.
			*/
			switch(item.second.index()) {
				case 0:
					yml << YAML::Key << "baseType" << YAML::Value << "int";
					yml << YAML::Key << "x" << YAML::Value << 1;
					yml << YAML::Key << "y" << YAML::Value << 1;
					yml << YAML::Key << "value" << YAML::Value << std::get<int>(item.second);
				case 1:
					yml << YAML::Key << "baseType" << YAML::Value << "uint";
					yml << YAML::Key << "x" << YAML::Value << 1;
					yml << YAML::Key << "y" << YAML::Value << 1;
					yml << YAML::Key << "value" << YAML::Value << std::get<unsigned int>(item.second);
				case 2:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 1;
					yml << YAML::Key << "y" << YAML::Value << 1;
					yml << YAML::Key << "value" << YAML::Value << std::get<float>(item.second);
				case 3:
					yml << YAML::Key << "baseType" << YAML::Value << "int";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec2<int> val = std::get<Vec2<int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::EndMap;
					}
				case 4:
					yml << YAML::Key << "baseType" << YAML::Value << "uint";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec2<unsigned int> val = std::get<Vec2<unsigned int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::EndMap;
					}
				case 5:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec2<float> val = std::get<Vec2<float>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::EndMap;
					}
				case 6:
					yml << YAML::Key << "baseType" << YAML::Value << "int";
					yml << YAML::Key << "x" << YAML::Value << 3;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec3<int> val = std::get<Vec3<int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::EndMap;
					}
				case 7:
					yml << YAML::Key << "baseType" << YAML::Value << "uint";
					yml << YAML::Key << "x" << YAML::Value << 3;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec3<unsigned int> val = std::get<Vec3<unsigned int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::EndMap;
					}
				case 8:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 3;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec3<float> val = std::get<Vec3<float>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::EndMap;
					}
				case 9:
					yml << YAML::Key << "baseType" << YAML::Value << "int";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec4<int> val = std::get<Vec4<int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::Key << "w" << YAML::Value << val.w;
						yml << YAML::EndMap;
					}
				case 10:
					yml << YAML::Key << "baseType" << YAML::Value << "uint";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec4<unsigned int> val = std::get<Vec4<unsigned int>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::Key << "w" << YAML::Value << val.w;
						yml << YAML::EndMap;
					}
				case 11:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 1;
					{
						Vec4<float> val = std::get<Vec4<float>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginMap;
						yml << YAML::Key << "x" << YAML::Value << val.x;
						yml << YAML::Key << "y" << YAML::Value << val.y;
						yml << YAML::Key << "z" << YAML::Value << val.z;
						yml << YAML::Key << "w" << YAML::Value << val.w;
						yml << YAML::EndMap;
					}
				case 12:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 2;
					{
						Matrix<float, 2, 2> val = std::get<Matrix<float, 2, 2>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 13:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 3;
					{
						Matrix<float, 2, 3> val = std::get<Matrix<float, 2, 3>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 14:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 2;
					yml << YAML::Key << "y" << YAML::Value << 4;
					{
						Matrix<float, 2, 4> val = std::get<Matrix<float, 2, 4>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 15:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 3;
					yml << YAML::Key << "y" << YAML::Value << 2;
					{
						Matrix<float, 3, 2> val = std::get<Matrix<float, 3, 2>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 16:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 3;
					yml << YAML::Key << "y" << YAML::Value << 3;
					{
						Matrix<float, 3, 3> val = std::get<Matrix<float, 3, 3>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 17:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 4;
					{
						Matrix<float, 4, 4> val = std::get<Matrix<float, 4, 4>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 18:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 2;
					{
						Matrix<float, 4, 2> val = std::get<Matrix<float, 4, 2>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 19:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 3;
					{
						Matrix<float, 4, 3> val = std::get<Matrix<float, 4, 3>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 20:
					yml << YAML::Key << "baseType" << YAML::Value << "float";
					yml << YAML::Key << "x" << YAML::Value << 4;
					yml << YAML::Key << "y" << YAML::Value << 4;
					{
						Matrix<float, 4, 4> val = std::get<Matrix<float, 4, 4>>(item.second);
						yml << YAML::Key << "value" << YAML::BeginSeq;
						for(const auto& row : val.data) {
							yml << YAML::BeginSeq;
							for(const auto& value : row) {
								yml << value;
							}
							yml << YAML::EndSeq;
						}
						yml << YAML::EndSeq;
					}
				case 21: {
					Material::TextureRef tr = std::get<Material::TextureRef>(item.second);
					yml << YAML::Key << "baseType" << YAML::Value << (tr.isCubemap ? "cubemap" : "tex2d");
					yml << YAML::Key << "x" << YAML::Value << 1;
					yml << YAML::Key << "y" << YAML::Value << 1;
					yml << YAML::Key << "value" << YAML::Value << tr.path;
				}
			}
			yml << YAML::EndMap;
		}
		yml << YAML::EndSeq;

		//Write output data
		out << yml.c_str() << std::flush;
	}

	void UnpackedEncoder::EncodeWorld(const World& world, std::ostream& out) {
		//Create YAML emitter
		YAML::Emitter yml;

		//Write skybox if present
		if(!world.skyboxRef.empty()) yml << YAML::Key << "skybox" << YAML::Value << world.skyboxRef;

		//Write camera data
		yml << YAML::Key << "cam" << YAML::BeginMap;
		yml << YAML::Key << "position" << YAML::BeginMap;
		yml << YAML::Key << "x" << YAML::Value << world.initialCamPos.x;
		yml << YAML::Key << "y" << YAML::Value << world.initialCamPos.y;
		yml << YAML::Key << "z" << YAML::Value << world.initialCamPos.z;
		yml << YAML::EndMap;
		yml << YAML::Key << "rotation" << YAML::BeginMap;
		yml << YAML::Key << "x" << YAML::Value << world.initialCamRot.x;
		yml << YAML::Key << "y" << YAML::Value << world.initialCamRot.y;
		yml << YAML::Key << "z" << YAML::Value << world.initialCamRot.z;
		yml << YAML::EndMap;
		yml << YAML::EndMap;

		//Write entity data
		yml << YAML::Key << "entities" << YAML::BeginSeq;
		for(const World::Entity& e : world.entities) {
			yml << YAML::BeginMap;

			//Write GUIDs
			yml << YAML::Key << "guid" << YAML::Value << e.guid;
			yml << YAML::Key << "parent" << YAML::Value << e.guid;

			//Write transform data
			yml << YAML::Key << "transform" << YAML::BeginMap;
			yml << YAML::Key << "position" << YAML::BeginMap;
			yml << YAML::Key << "x" << YAML::Value << e.initialPos.x;
			yml << YAML::Key << "y" << YAML::Value << e.initialPos.y;
			yml << YAML::Key << "z" << YAML::Value << e.initialPos.z;
			yml << YAML::EndMap;
			yml << YAML::Key << "rotation" << YAML::BeginMap;
			yml << YAML::Key << "x" << YAML::Value << e.initialRot.x;
			yml << YAML::Key << "y" << YAML::Value << e.initialRot.y;
			yml << YAML::Key << "z" << YAML::Value << e.initialRot.z;
			yml << YAML::EndMap;
			yml << YAML::Key << "scale" << YAML::BeginMap;
			yml << YAML::Key << "x" << YAML::Value << e.initialScale.x;
			yml << YAML::Key << "y" << YAML::Value << e.initialScale.y;
			yml << YAML::Key << "z" << YAML::Value << e.initialScale.z;
			yml << YAML::EndMap;
			yml << YAML::EndMap;

			//Write component data
			yml << YAML::Key << "components" << YAML::BeginSeq;
			for(const World::Component& c : e.components) {
				yml << YAML::BeginMap;

				yml << YAML::Key << "id" << YAML::Value;

				//This is kinda ugly, but this is how we have to deal with reflection data
				{
					try {
						er::None converted = er::serialization::binary::from_vector<er::None>(c.data).unwrap();
						std::string asYaml = er::serialization::yaml::to_string<er::None>(&converted).unwrap();
						YAML::Node node = YAML::Load(asYaml);
						yml << YAML::Key << "rfl" << node;
					} catch(...) {
						throw std::runtime_error("Component reflection data conversion failed");
					}
				}

				yml << YAML::EndMap;
			}
			yml << YAML::EndSeq;

			yml << YAML::EndMap;
		}
		yml << YAML::EndSeq;

		//Write output data
		out << yml.c_str() << std::flush;
	}
}