#pragma once

#include "glm/glm.hpp"

namespace Cacao {
	///@brief A vertex in a mesh
	struct Vertex {
		const glm::vec3 position; ///<The position in local space
		const glm::vec2 texCoords;///<The texture coordinates
		const glm::vec3 tangent;  ///<The right vector in tangent space
		const glm::vec3 bitangent;///<The front vector in tangent space
		const glm::vec3 normal;	  ///<The up vector in tangent space

		/**
		 * @brief Create a new vertex
		 *
		 * @param position The position in local space
		 * @param texCoords The texture coordinates (optional, defaults to {0, 0})
		 * @param tangent The right vector in tangent space (optional, defaults to {0, 0, 0})
		 * @param bitangent The front vector in tangent space (optional, defaults to {0, 0, 0})
		 * @param normal The up vector in tangent space (optional, defaults to {0, 0, 0})
		 */
		Vertex(glm::vec3 position, glm::vec2 texCoords = glm::vec2(0.0f), glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f), glm::vec3 normal = glm::vec3(0.0f))
		  : position(position), texCoords(texCoords), tangent(tangent), bitangent(bitangent), normal(normal) {}
	};
}