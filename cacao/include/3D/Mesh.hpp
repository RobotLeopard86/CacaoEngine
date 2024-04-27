#pragma once

#include "Vertex.hpp"
#include "Transform.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Asset.hpp"

#include <vector>
#include <future>

namespace Cacao {
	//Must be implemented per-rendering API
	class Mesh : public Asset {
	  public:
		Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);
		~Mesh() {
			if(compiled) Release();
			delete nativeData;
		}

		//Draw this mesh
		void Draw();
		//Compile the mesh into a usable form for drawing
		std::shared_future<void> Compile() override;
		//Release compiled assets from memory
		void Release() override;

		std::string GetType() override {
			return "MESH";
		}

	  private:
		std::vector<Vertex> vertices;
		std::vector<glm::uvec3> indices;

		NativeData* nativeData;
	};
}