#pragma once

#include "CheckException.hpp"

#include <functional>

#include "yaml-cpp/yaml.h"

namespace libcacaoformats {
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
	inline void ValidateYAMLNode(const YAML::Node& node, std::function<std::string(const YAML::Node&)> predicate, const std::string& context, const std::string& what) {
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
	inline void ValidateYAMLNode(const YAML::Node& node, YAML::NodeType::value type, const std::string& context, const std::string& what) {
		ValidateYAMLNode(node, [&type](const YAML::Node& node) { return (node.Type() == type ? "" : "Node is of improper type!"); }, context, what);
	}

	//Convenience wrapper for checking the type of the node AND doing a user predicate
	inline void ValidateYAMLNode(const YAML::Node& node, YAML::NodeType::value type, std::function<std::string(const YAML::Node&)> predicate, const std::string& context, const std::string& what) {
		ValidateYAMLNode(node, [&type, &predicate](const YAML::Node& node) { 
            if(std::string result = predicate(node); !result.empty()) CheckException(false, result);
            return (node.Type() == type ? "" : "Node is of improper type!"); }, context, what);
	}
}