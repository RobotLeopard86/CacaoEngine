#pragma once

#include "DllHelper.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Actor.hpp"

namespace Cacao {
	class CACAO_API MeshRenderer final : public Component {
	  public:
		MeshRenderer() {}

		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;

		virtual ~MeshRenderer() {}
	};
}