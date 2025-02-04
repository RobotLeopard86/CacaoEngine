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
		for(const auto& item : mat.values) {
			yml << YAML::BeginMap;
			yml << YAML::Key << "name" << YAML::Value << item.first;
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
		out << yml.c_str();
	}
}