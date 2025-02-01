#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include "yaml-cpp/yaml.h"
#include "er/serialization/yaml.h"
#include "er/serialization/binary.h"

namespace libcacaoformats {
	unsigned int stou(std::string value) {
		int i = std::stoi(value);
		if(i < 0) throw std::invalid_argument("Value is signed");
		return (unsigned int)i;
	}

	/**
	 * @brief Validates a YAML node
	 *
	 * @param node The node to examine
	 * @param predicate A function to execute for further validation, returning an empty string if check passes and an error message if not
	 * @param context Some context for the error message as to what is being parsed
	 * @param what A human-friendly description of what node is being parsed
	 *
	 * @throws std::runtime_error If a check fails
	 */
	void ValidateYAMLNode(const YAML::Node& node, std::function<std::string(const YAML::Node&)> predicate, const std::string& context, const std::string& what) {
		//Set up error message stream
		std::stringstream stream;
		stream << "While parsing " << context << ", " << what << " node is invalid: ";

		//Check that the node exists and is the right type
		CheckException(node.IsDefined(), ((stream << "Node does not exist"), stream.str()));

		//Run the user predicate
		std::string out = predicate(node);
		if(out.empty()) return;
		CheckException(false, ((stream << out), stream.str()));
	}

	//Convenience wrapper for checking the type of the node
	void ValidateYAMLNode(const YAML::Node& node, YAML::NodeType::value type, const std::string& context, const std::string& what) {
		ValidateYAMLNode(node, [&type](const YAML::Node& node) { return (node.Type() == type ? "" : "Node is of improper type!"); }, context, what);
	}

	//Convenience wrapper for checking the type of the node AND doing a user predicate
	void ValidateYAMLNode(const YAML::Node& node, YAML::NodeType::value type, std::function<std::string(const YAML::Node&)> predicate, const std::string& context, const std::string& what) {
		ValidateYAMLNode(node, [&type, &predicate](const YAML::Node& node) { 
            if(std::string result = predicate(node); !result.empty()) CheckException(false, result);
            return (node.Type() == type ? "" : "Node is of improper type!"); }, context, what);
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
		YAML::Node keysRoot = root["keys"];
		ValidateYAMLNode(keysRoot, YAML::NodeType::Sequence, "unpacked material data", "key list");
		for(const YAML::Node& node : keysRoot) {
			constexpr std::array<const char*, 5> okTypes = {{"int", "uint", "float", "tex2d", "cubemap"}};
			ValidateYAMLNode(node["name"], YAML::NodeType::value::Scalar, "unpacked material data key", "key name");
			ValidateYAMLNode(node["baseType"], YAML::NodeType::value::Scalar, [](const YAML::Node& node2) {
				auto it = std::find(okTypes.begin(), okTypes.end(), node2.Scalar().c_str());
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
			ValidateYAMLNode(node["value"], [&node, &value](const YAML::Node& node2) {
				int idx = 0;
				for(; idx < okTypes.size(); idx++) {
					if(node["baseType"].Scalar().compare(okTypes[idx]) == 0) break;
				}
				if(idx > 4) return "Invalid base type";
				Vec2<int> size;
				try {
					size.x = std::stoi(node["x"].Scalar().c_str(), nullptr);
					return (size.x > 0 && size.x < 5) ? "" : "Invalid x size value";
				} catch(...) {
					return "Unable to convert x size value to integer";
				}
				if(idx >= 3 && size.x != 1) return "Invalid x size value for texture";
				try {
					size.y = std::stoi(node["y"].Scalar().c_str(), nullptr);
					return (size.y > 0 && size.y < 5) ? "" : "Invalid y size value";
				} catch(...) {
					return "Unable to convert y size value to integer";
				}
				if(idx >= 3 && size.y != 1) return "Invalid y size value for texture";
				if(idx >= 3) {
					value = node2.Scalar();
					return "";
				}
				if(size.x == 1 && size.y != 1) return "Invalid y size for an x size 1";
				if(idx != 2 && size.y > 1) return "Invalid y size for non-float value";
				int dims = (4 * size.y) - (4 - size.x);
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
									if(!(node2.IsSequence() && node2.size() != 2)) return "Non-2-row matrix supplied for 2x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x2 matrix";
									Matrix<float, 2, 2> v;
									v.data[0][0] = std::strtof(node2[0][0].Scalar().c_str(), nullptr);
									v.data[0][1] = std::strtof(node2[0][1].Scalar().c_str(), nullptr);
									v.data[1][0] = std::strtof(node2[1][0].Scalar().c_str(), nullptr);
									v.data[1][1] = std::strtof(node2[1][1].Scalar().c_str(), nullptr);
									value = v;
									return "";
								}
								case 7: {
									if(!(node2.IsSequence() && node2.size() != 3)) return "Non-3-row matrix supplied for 2x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 2 && node2[2][0].IsScalar() && node2[2][1].IsScalar())) return "Non-2 float sequence supplied at row 3 of 2x3 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 4)) return "Non-4-row matrix supplied for 2x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 2 && node2[0][0].IsScalar() && node2[0][1].IsScalar())) return "Non-2 float sequence supplied at row 1 of 2x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 2 && node2[1][0].IsScalar() && node2[1][1].IsScalar())) return "Non-2 float sequence supplied at row 2 of 2x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 2 && node2[2][0].IsScalar() && node2[2][1].IsScalar())) return "Non-2 float sequence supplied at row 3 of 2x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() != 2 && node2[3][0].IsScalar() && node2[3][1].IsScalar())) return "Non-2 float sequence supplied at row 4 of 2x4 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 2)) return "Non-2-row matrix supplied for 3x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x2 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 3)) return "Non-3-row matrix supplied for 3x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 3 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node2[2][2].IsScalar())) return "Non-3 float sequence supplied at row 3 of 3x3 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 4)) return "Non-4-row matrix supplied for 3x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 3 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node2[0][2].IsScalar())) return "Non-3 float sequence supplied at row 1 of 3x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 3 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node2[1][2].IsScalar())) return "Non-3 float sequence supplied at row 2 of 3x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 3 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node2[2][2].IsScalar())) return "Non-3 float sequence supplied at row 3 of 3x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() != 3 && node2[3][0].IsScalar() && node2[3][1].IsScalar() && node2[3][2].IsScalar())) return "Non-3 float sequence supplied at row 4 of 3x4 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 2)) return "Non-2-row matrix supplied for 4x2 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x2 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x2 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 3)) return "Non-3-row matrix supplied for 4x3 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x3 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x3 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 4 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node[2][2].IsScalar() && node[2][3].IsScalar())) return "Non-4 float sequence supplied at row 3 of 4x3 matrix";
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
									if(!(node2.IsSequence() && node2.size() != 4)) return "Non-4-row matrix supplied for 4x4 matrix key";
									if(!(node2[0].IsSequence() && node2[0].size() != 4 && node2[0][0].IsScalar() && node2[0][1].IsScalar() && node[0][2].IsScalar() && node[0][3].IsScalar())) return "Non-4 float sequence supplied at row 1 of 4x4 matrix";
									if(!(node2[1].IsSequence() && node2[1].size() != 4 && node2[1][0].IsScalar() && node2[1][1].IsScalar() && node[1][2].IsScalar() && node[1][3].IsScalar())) return "Non-4 float sequence supplied at row 2 of 4x4 matrix";
									if(!(node2[2].IsSequence() && node2[2].size() != 4 && node2[2][0].IsScalar() && node2[2][1].IsScalar() && node[2][2].IsScalar() && node[2][3].IsScalar())) return "Non-4 float sequence supplied at row 3 of 4x4 matrix";
									if(!(node2[3].IsSequence() && node2[3].size() != 4 && node2[3][0].IsScalar() && node2[3][1].IsScalar() && node[3][2].IsScalar() && node[3][3].IsScalar())) return "Non-4 float sequence supplied at row 4 of 4x4 matrix";
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
							}
						default:
							return "Invalid base type";
					}
				} catch(const std::exception& e) {
					return e.what();
				} }, "unpacked material data key", "key value");
			out.values.insert_or_assign(node["name"].Scalar(), value);
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
		if(sky) ValidateYAMLNode(sky, YAML::NodeType::value::Scalar, "unpacked world data", "Skybox asset path is not a string");
		out.skyboxRef = sky.Scalar();
		out.imports.push_back(out.skyboxRef);
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
		ValidateYAMLNode(root["entities"], YAML::NodeType::value::Sequence, "unpacked world data", "Entities list is not a list");
		for(const YAML::Node& e : root["entities"]) {
			World::Entity entity;

			YAML::Node name = e["name"];
			ValidateYAMLNode(name, YAML::NodeType::value::Scalar, "unpacked world entity", "Name is not a string");
			entity.name = name.Scalar();
		}

		//Return result
	}
}