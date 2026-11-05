#pragma once

#include "DllHelper.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Actor.hpp"

namespace Cacao {
	class CACAO_API ASTRA_REFLECT MeshRenderer final : public Component {
	  public:
		MeshRenderer() {}

		void OnEnable() override;
		void OnDisable() override;

		ASTRA_IGNORE std::shared_ptr<Mesh> mesh;
		ASTRA_IGNORE std::shared_ptr<Material> material;

		ASTRASETUP(MeshRenderer)
		virtual ~MeshRenderer() {}
	};
}