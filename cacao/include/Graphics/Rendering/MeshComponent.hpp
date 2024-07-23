#pragma once

#include "World/Component.hpp"

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"

#include <string>

namespace Cacao {
	class MeshComponent : public Component {
	  public:
		std::string GetKind() override {
			return "MESH";
		}

		AssetHandle<Mesh> mesh;
		std::shared_ptr<Material> mat;
	};
}