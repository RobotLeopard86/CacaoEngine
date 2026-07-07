#include "Cacao/Mesh.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Mesh.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "PALConfigurables.hpp"

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex>&& vtx, std::vector<glm::uvec3>&& idx, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Mesh>(addr), "Resource address is malformed!");
		Check<BadValueException>(!vtx.empty() && !idx.empty(), "Cannot construct a mesh with empty data!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->vertices = std::move(vtx);
		impl->indices = std::move(idx);
	}

	std::shared_ptr<Mesh> Mesh::Create(std::vector<Vertex>&& vtx, std::vector<glm::uvec3>&& idx, const std::string& addr) {
		std::shared_ptr<Mesh> ptr(new Mesh(std::move(vtx), std::move(idx), addr));
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Mesh::~Mesh() {
		if(baked) Discard();
	}

	Mesh::Mesh(Mesh&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy baking state
		baked = other.baked;
		other.baked = false;

		//Blank out other asset address
		other.address = "";
	}

	Mesh& Mesh::operator=(Mesh&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Baking state
		baked = other.baked;
		other.baked = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Mesh::Bake() {
		Check<BadBakeStateException>(!baked, "Cannot bake a baked mesh!");

		impl->Bake(baked);
	}

	void Mesh::Discard() {
		Check<BadBakeStateException>(baked, "Cannot discard baked representation of an unbaked mesh; it does not exist!");

		baked = false;
		impl->Discard();
	}
}