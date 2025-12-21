#pragma once

#include "Cacao/Mesh.hpp"

#include <optional>

namespace Cacao {
	class Mesh::Impl {
	  public:
		virtual void Realize(bool& success) = 0;
		virtual void DropRealized() = 0;

		std::vector<Vertex> vertices;
		std::vector<glm::uvec3> indices;

		virtual ~Impl() = default;
	};
}