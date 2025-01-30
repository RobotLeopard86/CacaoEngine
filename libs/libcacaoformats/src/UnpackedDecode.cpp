#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include "yaml-cpp/yaml.h"
#include "er/serialization/yaml.h"

namespace libcacaoformats {
	//Normally I wouldn't use a Doxygen comment here, but it's formatted nicely, so
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

		//You will see a lot of the comma operator. Look it up if you don't know what it does.

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
}