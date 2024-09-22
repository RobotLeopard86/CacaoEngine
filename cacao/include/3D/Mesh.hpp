#pragma once

#include "Vertex.hpp"
#include "Transform.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Asset.hpp"

#include <vector>
#include <future>

namespace Cacao {
	//Must be implemented per-rendering API
	class Mesh final : public Asset {
	  public:
		Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);
		~Mesh() final {
			if(compiled) Release();
		}

		//Draw this mesh
		void Draw();
		//Compile the mesh into a usable form for drawing
		std::shared_future<void> Compile() override;
		//Release compiled assets from memory
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() override {
			return "MESH";
		}

	  private:
		//Backend-implemented data type
		struct MeshData;

		std::vector<Vertex> vertices;
		std::vector<glm::uvec3> indices;

		std::shared_ptr<MeshData> nativeData;
	};
}