#pragma once

#include "World/Component.hpp"
#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "Core/DllHelper.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief Mesh renderer component
	 */
	class CACAO_API MeshComponent : public Component {
	  public:
		///@brief Gets the type of this componet. Needed for safe downcasting from Component
		std::string GetKind() override {
			return "MESH";
		}

		AssetHandle<Mesh> mesh;		  ///<The mesh to render
		std::shared_ptr<Material> mat;///<The material to render the mesh with
	};
}