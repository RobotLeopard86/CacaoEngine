#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"
#include "YAMLValidate.hpp"

#include "yaml-cpp/yaml.h"

#include <iostream>

namespace libcacaoformats {
	unsigned int stou(std::string value) {
		int i = std::stoi(value);
		if(i < 0) throw std::invalid_argument("Value is signed");
		return (unsigned int)i;
	}

	std::array<ImageBuffer, 6> UnpackedDecoder::DecodeCubemap(std::istream& data, std::function<std::istream(const std::string&)> loader) {
		CheckException(data.good(), "Data stream for unpacked cubemap is invalid!");

		//Load YAML
		YAML::Node root;
		try {
			root = YAML::Load(data);
		} catch(...) {
			CheckException(false, "Failed to parse unpacked cubemap data stream!");
		}

		//Validate structure
		YAML::Node posX = root["positiveX"];
		ValidateYAMLNode(posX, YAML::NodeType::value::Scalar, "unpacked cubemap data", "positive X face");
		YAML::Node negX = root["negativeX"];
		ValidateYAMLNode(negX, YAML::NodeType::value::Scalar, "unpacked cubemap data", "negative X face");
		YAML::Node posY = root["positiveY"];
		ValidateYAMLNode(posY, YAML::NodeType::value::Scalar, "unpacked cubemap data", "positive Y face");
		YAML::Node negY = root["negativeY"];
		ValidateYAMLNode(negY, YAML::NodeType::value::Scalar, "unpacked cubemap data", "negative Y face");
		YAML::Node posZ = root["positiveZ"];
		ValidateYAMLNode(posZ, YAML::NodeType::value::Scalar, "unpacked cubemap data", "positive Z face");
		YAML::Node negZ = root["negativeZ"];
		ValidateYAMLNode(negZ, YAML::NodeType::value::Scalar, "unpacked cubemap data", "negative Z face");

		//Get data streams for faces
		std::istream pxs = loader(posX.Scalar());
		CheckException(pxs.good(), "Positive X face data input stream is invalid!");
		std::istream nxs = loader(negX.Scalar());
		CheckException(nxs.good(), "Negative X face data input stream is invalid!");
		std::istream pys = loader(posY.Scalar());
		CheckException(pys.good(), "Positive Y face data input stream is invalid!");
		std::istream nys = loader(negY.Scalar());
		CheckException(nys.good(), "Negative Y face data input stream is invalid!");
		std::istream pzs = loader(posZ.Scalar());
		CheckException(pzs.good(), "Positive Z face data input stream is invalid!");
		std::istream nzs = loader(negZ.Scalar());
		CheckException(nzs.good(), "Negative Z face data input stream is invalid!");

		//Decode image data
		std::array<ImageBuffer, 6> out;
		out[0] = DecodeImage(pxs);
		out[1] = DecodeImage(nxs);
		out[2] = DecodeImage(pys);
		out[3] = DecodeImage(nys);
		out[4] = DecodeImage(pzs);
		out[5] = DecodeImage(nzs);

		//Return result
		return out;
	}

	Shader UnpackedDecoder::DecodeShader(std::istream& data, std::function<std::istream(const std::string&)> loader) {
		CheckException(data.good(), "Data stream for unpacked shader is invalid!");

		//Load YAML
		YAML::Node root;
		try {
			root = YAML::Load(data);
		} catch(...) {
			CheckException(false, "Failed to parse unpacked shader data stream!");
		}

		//Validate structure
		YAML::Node vert = root["vertex"];
		ValidateYAMLNode(vert, YAML::NodeType::value::Scalar, "unpacked shader data", "vertex shader stage SPIR-V");
		YAML::Node frag = root["fragment"];
		ValidateYAMLNode(frag, YAML::NodeType::value::Scalar, "unpacked shader data", "fragment shader stage SPIR-V");

		//Get data streams for stages
		std::istream vertstream = loader(vert.Scalar());
		CheckException(vertstream.good(), "Vertex shader stage data input stream is invalid!");
		std::istream fragstream = loader(frag.Scalar());
		CheckException(fragstream.good(), "Fragment shader stage data input stream is invalid!");

		//Extract SPIR-V
		Shader out = {};
		out.vertexSPV = [&vertstream]() {
			try {
				//Grab size
				vertstream.exceptions(std::ios::failbit | std::ios::badbit);
				vertstream.seekg(0, std::ios::end);
				auto size = vertstream.tellg();
				vertstream.seekg(0, std::ios::beg);

				//Read data
				std::vector<uint32_t> contents(size);
				vertstream.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) { throw ios_failure; }
				CheckException(false, "Failed to read vertex shader stage data stream!");
				return std::vector<uint32_t>();//This will never be reached because of the exception, but the compiler would throw a warning if this wasn't here
			}
		}();
		out.fragmentSPV = [&fragstream]() {
			try {
				//Grab size
				fragstream.exceptions(std::ios::failbit | std::ios::badbit);
				fragstream.seekg(0, std::ios::end);
				auto size = fragstream.tellg();
				fragstream.seekg(0, std::ios::beg);

				//Read data
				std::vector<uint32_t> contents(size);
				fragstream.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) { throw ios_failure; }
				CheckException(false, "Failed to read vfragmentshader stage data stream!");
				return std::vector<uint32_t>();//This will never be reached because of the exception, but the compiler would throw a warning if this wasn't here
			}
		}();

		//Return result
		return out;
	}

	Material UnpackedDecoder::DecodeMaterial(std::istream& data) {
		CheckException(data.good(), "Data stream for unpacked material is invalid!");

		//Load YAML
		YAML::Node root;
		try {
			root = YAML::Load(data);
		} catch(...) {
			CheckException(false, "Failed to parse unpacked material data stream!");
		}

		Material out;

		//Validate and parse structure
		YAML::Node shader = root["shader"];
		ValidateYAMLNode(shader, YAML::NodeType::value::Scalar, "unpacked material data", "shader reference");
		out.shader = shader.Scalar();
		YAML::Node dataRoot = root["data"];
		ValidateYAMLNode(dataRoot, YAML::NodeType::Sequence, "unpacked material data", "key list");
		for(const YAML::Node& node : dataRoot) {
			constexpr std::array<const char*, 5> okTypes = {{"int", "uint", "float", "tex2d", "cubemap"}};
			ValidateYAMLNode(node["name"], YAML::NodeType::value::Scalar, "unpacked material data key", "key name");
			ValidateYAMLNode(node["baseType"], YAML::NodeType::value::Scalar, [&okTypes](const YAML::Node& node2) {
				auto it = std::find(okTypes.begin(), okTypes.end(), std::string(node2.Scalar().c_str()));
				return (it != okTypes.end() ? "" : "Invalid base type"); }, "unpacked material data key", "key base type");
			ValidateYAMLNode(node["x"], YAML::NodeType::value::Scalar, [](const YAML::Node& node2) {
				try {
					int val = std::stoi(node2.Scalar().c_str(), nullptr);
					return (val > 0 && val < 5) ? "" : "Invalid size value";
				} catch(...) {
					return "Unable to convert value to integer";
				} }, "unpacked material data key", "key x size");
			ValidateYAMLNode(node["y"], YAML::NodeType::value::Scalar, [](const YAML::Node& node2) {
				try {
					int val = std::stoi(node2.Scalar().c_str(), nullptr);
					return (val > 0 && val < 5) ? "" : "Invalid size value";
				} catch(...) {
					return "Unable to convert value to integer";
				} }, "unpacked material data key", "key y size");
			Material::ValueContainer value;
			const auto valFunc = [&node, &value, &okTypes](const YAML::Node& node2) {
				int idx = 0;
				for(; idx < okTypes.size(); idx++) {
					if(node["baseType"].Scalar().compare(okTypes[idx]) == 0) break;
				}
				if(idx > 4) return "Invalid base type";
				Vec2<int> size;
				try {
					size.x = std::stoi(node["x"].Scalar().c_str(), nullptr);
					if(size.x <= 0 || size.x >= 5) return "Invalid x size value";
				} catch(...) {
					return "Unable to convert x size value to integer";
				}
				if(idx >= 3 && size.x != 1) return "Invalid x size value for texture";
				try {
					size.y = std::stoi(node["y"].Scalar().c_str(), nullptr);
					if(size.y <= 0 || size.y >= 5) return "Invalid y size value";
				} catch(...) {
					return "Unable to convert y size value to integer";
				}
				if(idx >= 3 && size.y != 1) return "Invalid y size value for texture";
				if(idx >= 3) {
					value = Material::TextureRef {.path = node2.Scalar(), .isCubemap = idx == 5};
					return "";
				}
				if(!(size.y != 1 || (size.x == 1 && size.y == 1))) return "Invalid y size for an x size 1";
				if(idx != 2 && size.x > 1) return "Invalid y size for non-float value";
				int dims = (4 * size.x) - (4 - size.y);
				try {
					switch(idx) {
						case 0:
							switch(dims) {
								case 1:
									if(!node2.IsScalar()) return "Non-integer value supplied for integer key";
									value = std::stoi(node2.Scalar().c_str(), nullptr);
									return "";
								case 2: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar())) return "Expected 'x' and 'y' scalar values for two-component integer vector";
									Vec2<int> v;
									v.x = std::stoi(node2["x"].Scalar().c_str());
									v.y = std::stoi(node2["y"].Scalar().c_str());
									value = v;
									return "";
								}
								case 3: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for three-component integer vector";
									Vec3<int> v;
									v.x = std::stoi(node2["x"].Scalar().c_str());
									v.y = std::stoi(node2["y"].Scalar().c_str());
									v.z = std::stoi(node2["z"].Scalar().c_str());
									value = v;
									return "";
								}
								case 4: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar() && node2["w"].IsScalar())) return "Expected 'x', 'y', 'z', and 'w' scalar values for four-component integer vector";
									Vec4<int> v;
									v.x = std::stoi(node2["x"].Scalar().c_str());
									v.y = std::stoi(node2["y"].Scalar().c_str());
									v.z = std::stoi(node2["z"].Scalar().c_str());
									v.w = std::stoi(node2["w"].Scalar().c_str());
									value = v;
									return "";
								}
							}
							break;
						case 1:
							switch(dims) {
								case 1:
									if(!node2.IsScalar()) return "Non-integer value supplied for unsigned integer key";
									value = stou(node2.Scalar().c_str());
									return "";
								case 2: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar())) return "Expected 'x' and 'y' scalar values for two-component unsigned integer vector";
									Vec2<unsigned int> v;
									v.x = stou(node2["x"].Scalar().c_str());
									v.y = stou(node2["y"].Scalar().c_str());
									value = v;
									return "";
								}
								case 3: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for three-component unsigned integer vector";
									Vec3<unsigned int> v;
									v.x = stou(node2["x"].Scalar().c_str());
									v.y = stou(node2["y"].Scalar().c_str());
									v.z = stou(node2["z"].Scalar().c_str());
									value = v;
									return "";
								}
								case 4: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar() && node2["w"].IsScalar())) return "Expected 'x', 'y', 'z', and 'w' scalar values for four-component unsigned integer vector";
									Vec4<unsigned int> v;
									v.x = stou(node2["x"].Scalar().c_str());
									v.y = stou(node2["y"].Scalar().c_str());
									v.z = stou(node2["z"].Scalar().c_str());
									v.w = stou(node2["w"].Scalar().c_str());
									value = v;
									return "";
								}
							}
							break;
						case 2:
							switch(dims) {
								case 1:
									if(!node2.IsScalar()) return "Non-float value supplied for float key";
									value = std::strtof(node2.Scalar().c_str(), nullptr);
									return "";
								case 2: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar())) return "Expected 'x' and 'y' scalar values for two-component float vector";
									Vec2<float> v;
									v.x = std::strtof(node2["x"].Scalar().c_str(), nullptr);
									v.y = std::strtof(node2["y"].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 3: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for three-component float vector";
									Vec3<float> v;
									v.x = std::strtof(node2["x"].Scalar().c_str(), nullptr);
									v.y = std::strtof(node2["y"].Scalar().c_str(), nullptr);
									v.z = std::strtof(node2["z"].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 4: {
									if(!(node2.IsMap() && node2["x"].IsScalar() && node2["y"].IsScalar() && node2["z"].IsScalar() && node2["w"].IsScalar())) return "Expected 'x', 'y', 'z', and 'w' scalar values for four-component float vector";
									Vec4<float> v;
									v.x = std::strtof(node2["x"].Scalar().c_str(), nullptr);
									v.y = std::strtof(node2["y"].Scalar().c_str(), nullptr);
									v.z = std::strtof(node2["z"].Scalar().c_str(), nullptr);
									v.w = std::strtof(node2["w"].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 6: {
									if(!(node2.IsSequence() && node2.size() == 2)) return "Non-2-row matrix supplied for 2x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x2 matrix";
									Matrix<float, 2, 2> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 7: {
									if(!(node2.IsSequence() && node2.size() == 3)) return "Non-3-row matrix supplied for 2x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 2 && node2[2][0].IsScalar() && node2[2][1].IsScalar())) return "Non-2 float sequence supplied at row 3 of 2x3 matrix";
									Matrix<float, 2, 3> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 8: {
									if(!(node2.IsSequence() && node2.size() == 4)) return "Non-4-row matrix supplied for 2x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 2 && node2[2][0].IsScalar() && node2[2][1].IsScalar())) return "Non-2 float sequence supplied at row 3 of 2x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() == 2 && node2[3][0].IsScalar() && node2[3][1].IsScalar())) return "Non-2 float sequence supplied at row 4 of 2x4 matrix";
									Matrix<float, 2, 4> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									v.data[3][0] = std::strtof(node2[3][0].Scalar().c_str(), nullptr);
									v.data[3][1] = std::strtof(node2[3][1].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 10: {
									if(!(node2.IsSequence() && node2.size() == 2)) return "Non-2-row matrix supplied for 3x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x2 matrix";
									Matrix<float, 3, 2> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][2].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 11: {
									if(!(node2.IsSequence() && node2.size() == 3)) return "Non-3-row matrix supplied for 3x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 3 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node2[2][2].IsScalar())) return "Non-3 float sequence supplied at row 3 of 3x3 matrix";
									Matrix<float, 3, 3> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									v.data[2][2] = std::strtof(node2[2][2].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 12: {
									if(!(node2.IsSequence() && node2.size() == 4)) return "Non-4-row matrix supplied for 3x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 3 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node2[2][2].IsScalar())) return "Non-3 float sequence supplied at row 3 of 3x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() == 3 && node2[3][0].IsScalar() && node2[3][1].IsScalar() && node2[3][2].IsScalar())) return "Non-3 float sequence supplied at row 4 of 3x4 matrix";
									Matrix<float, 3, 4> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][2].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									v.data[2][2] = std::strtof(node2[2][2].Scalar().c_str(), nullptr);
									v.data[3][0] = std::strtof(node2[3][0].Scalar().c_str(), nullptr);
									v.data[3][1] = std::strtof(node2[3][1].Scalar().c_str(), nullptr);
									v.data[3][2] = std::strtof(node2[3][2].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 14: {
									if(!(node2.IsSequence() && node2.size() == 2)) return "Non-2-row matrix supplied for 4x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x2 matrix";
									Matrix<float, 4, 2> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[0][3] = std::strtof(node2[0][3].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][2].Scalar().c_str(), nullptr);
									v.data[1][3] = std::strtof(node2[1][3].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 15: {
									if(!(node2.IsSequence() && node2.size() == 3)) return "Non-3-row matrix supplied for 4x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 4 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node[2][2].IsScalar() && node[2][3].IsScalar())) return "Non-4 float sequence supplied at row 3 of 4x3 matrix";
									Matrix<float, 4, 3> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[0][3] = std::strtof(node2[0][3].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][3] = std::strtof(node2[1][3].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									v.data[2][2] = std::strtof(node2[2][2].Scalar().c_str(), nullptr);
									v.data[2][3] = std::strtof(node2[2][3].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 16: {
									if(!(node2.IsSequence() && node2.size() == 4)) return "Non-4-row matrix supplied for 4x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() == 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() == 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() == 4 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node[2][2].IsScalar() && node[2][3].IsScalar())) return "Non-4 float sequence supplied at row 3 of 4x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() == 4 && node2[3][0].IsScalar() && node2[3][1].IsScalar() && node[3][2].IsScalar() && node[3][3].IsScalar())) return "Non-4 float sequence supplied at row 4 of 4x4 matrix";
									Matrix<float, 4, 4> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[0][2] = std::strtof(node2[0][2].Scalar().c_str(), nullptr);
									v.data[0][3] = std::strtof(node2[0][3].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									v.data[1][2] = std::strtof(node2[1][2].Scalar().c_str(), nullptr);
									v.data[1][3] = std::strtof(node2[1][3].Scalar().c_str(), nullptr);
									v.data[2][0] = std::strtof(node2[2][0].Scalar().c_str(), nullptr);
									v.data[2][1] = std::strtof(node2[2][1].Scalar().c_str(), nullptr);
									v.data[2][2] = std::strtof(node2[2][2].Scalar().c_str(), nullptr);
									v.data[2][3] = std::strtof(node2[2][3].Scalar().c_str(), nullptr);
									v.data[3][0] = std::strtof(node2[3][0].Scalar().c_str(), nullptr);
									v.data[3][1] = std::strtof(node2[3][1].Scalar().c_str(), nullptr);
									v.data[3][2] = std::strtof(node2[3][2].Scalar().c_str(), nullptr);
									v.data[3][3] = std::strtof(node2[3][3].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								default: break;
							}
						default:
							return "Invalid base type";
					}
				} catch(const std::exception& e) {
					return e.what();
				}
				return "";
			};
			try {
				ValidateYAMLNode(node["value"], valFunc, "unpacked material data key", "key value");
				out.keys.insert_or_assign(node["name"].Scalar(), value);
			} catch(...) {
				std::rethrow_exception(std::current_exception());
			}
		}

		//Return result
		return out;
	}

	World UnpackedDecoder::DecodeWorld(std::istream& data) {
		CheckException(data.good(), "Data stream for unpacked material is invalid!");

		//Load YAML
		YAML::Node root;
		try {
			root = YAML::Load(data);
		} catch(...) {
			CheckException(false, "Failed to parse unpacked material data stream!");
		}

		//Validate and parse structure
		World out;
		YAML::Node sky = root["skybox"];
		if(sky) {
			ValidateYAMLNode(sky, YAML::NodeType::value::Scalar, "unpacked world data", "skybox asset path");
			out.skyboxRef = sky.Scalar();
		} else {
			out.skyboxRef = "";
		}
		YAML::Node cam = root["cam"];
		ValidateYAMLNode(cam, YAML::NodeType::value::Map, [&out](const YAML::Node& node) {
			YAML::Node p = node["position"], r = node["rotation"];
			try {
				if(!(p.IsMap() && p["x"].IsScalar() && p["y"].IsScalar() && p["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for camera initial position";
				out.initialCamPos.x = std::strtof(p["x"].Scalar().c_str(), nullptr);
				out.initialCamPos.y = std::strtof(p["y"].Scalar().c_str(), nullptr);
				out.initialCamPos.z = std::strtof(p["z"].Scalar().c_str(), nullptr);
				if(!(p.IsMap() && r["x"].IsScalar() && r["y"].IsScalar() && r["z"].IsScalar())) return "Expected 'x', 'y', and 'z' scalar values for camera initial rotation";
				out.initialCamRot.x = std::strtof(r["x"].Scalar().c_str(), nullptr);
				out.initialCamRot.y = std::strtof(r["y"].Scalar().c_str(), nullptr);
				out.initialCamRot.z = std::strtof(r["z"].Scalar().c_str(), nullptr);
			} catch(...) {
				return "Non-float value found in camera initial state";
			}
			return ""; }, "unpacked world data", "initial camera state");
		ValidateYAMLNode(root["entities"], YAML::NodeType::value::Sequence, "unpacked world data", "entities list");
		for(const YAML::Node& e : root["entities"]) {
			World::Entity entity;

			YAML::Node name = e["name"];
			ValidateYAMLNode(name, YAML::NodeType::value::Scalar, "unpacked world entity", "entity name");
			entity.name = name.Scalar();

			YAML::Node guid = e["guid"];
			ValidateYAMLNode(guid, YAML::NodeType::value::Scalar, [](const YAML::Node& node) {
				for(char c : node.Scalar()) {
					if(c == '-') continue;
					if(c < 48 || c > 102 || (c > 57 && c < 97)) {
						std::stringstream ss;
						ss << "Invalid GUID character \"" << c << "\"";
						return ss.str();
					}
				}
				return std::string(""); }, "unpacked world entity", "GUID");
			entity.guid = xg::Guid(guid.Scalar());

			YAML::Node parentGUID = e["parent"];
			ValidateYAMLNode(parentGUID, YAML::NodeType::value::Scalar, [](const YAML::Node& node) {
				for(char c : node.Scalar()) {
					if(c == '-') continue;
					if(c < 48 || c > 102 || (c > 57 && c < 97)) {
						std::stringstream ss;
						ss << "Invalid GUID character \"" << c << "\"";
						return ss.str();
					}
				}
				return std::string(""); }, "unpacked world entity", "parent GUID");
			entity.parentGUID = xg::Guid(parentGUID.Scalar());

			YAML::Node transform = e["transform"];
			ValidateYAMLNode(transform, YAML::NodeType::value::Map, "unpacked world entity", "initial transform");

			YAML::Node pos = transform["position"];
			ValidateYAMLNode(pos, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "unpacked world entity transform", "position property");
			entity.initialPos.x = std::strtof(pos["x"].Scalar().c_str(), nullptr);
			entity.initialPos.y = std::strtof(pos["y"].Scalar().c_str(), nullptr);
			entity.initialPos.z = std::strtof(pos["z"].Scalar().c_str(), nullptr);

			YAML::Node rot = transform["rotation"];
			ValidateYAMLNode(rot, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "unpacked world entity transform", "rotation property");
			entity.initialRot.x = std::strtof(rot["x"].Scalar().c_str(), nullptr);
			entity.initialRot.y = std::strtof(rot["y"].Scalar().c_str(), nullptr);
			entity.initialRot.z = std::strtof(rot["z"].Scalar().c_str(), nullptr);

			YAML::Node scl = transform["scale"];
			ValidateYAMLNode(scl, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["x"].IsScalar()) return "X value is not a scalar";
				if(!node["y"].IsScalar()) return "Y value is not a scalar";
				if(!node["z"].IsScalar()) return "Z value is not a scalar";
				return ""; }, "unpacked world entity transform", "scale property");
			entity.initialScale.x = std::strtof(scl["x"].Scalar().c_str(), nullptr);
			entity.initialScale.y = std::strtof(scl["y"].Scalar().c_str(), nullptr);
			entity.initialScale.z = std::strtof(scl["z"].Scalar().c_str(), nullptr);

			YAML::Node components = e["components"];
			ValidateYAMLNode(components, YAML::NodeType::value::Sequence, "unpacked world entity", "component list");
			for(const YAML::Node& c : components) {
				World::Component component;

				YAML::Node id = c["id"];
				ValidateYAMLNode(id, YAML::NodeType::value::Scalar, "unpacked world entity component", "component ID");
				component.typeID = id.Scalar();

				YAML::Node rfl = c["rfl"];
				ValidateYAMLNode(rfl, [](const YAML::Node& node) { return (node.IsDefined() ? "" : "Reflection data doesn't exist"); }, "unpacked world entity component", "component reflection data");

				//We use this to get the reflection data node back out as a string
				YAML::Emitter emitter;
				emitter << rfl;
				component.reflection = emitter.c_str();

				entity.components.push_back(component);
			}

			out.entities.push_back(entity);
		}

		//Return result
		return out;
	}
}