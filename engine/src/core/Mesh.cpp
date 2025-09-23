#include "Cacao/Mesh.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/ThreadPool.hpp"
#include "impl/Mesh.hpp"
#include "PALConfigurables.hpp"

#include <future>

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

	Mesh::~Mesh() {
		if(realized) DropRealized();
	}

	Mesh::Mesh(Mesh&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy realization state
		realized = other.realized;
		other.realized = false;

		//Blank out other asset address
		other.address = "";
	}

	Mesh& Mesh::operator=(Mesh&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Realization state
		realized = other.realized;
		other.realized = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Mesh::Realize() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized mesh!");

		if(impl->DoWaitAsyncForSync()) {
			impl->Realize(realized).value().get();
		} else {
			impl->Realize(realized);
		}
	}

	std::shared_future<void> Mesh::RealizeAsync() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized mesh!");

		if(impl->DoWaitAsyncForSync()) {
			return impl->Realize(realized).value();
		} else {
			return ThreadPool::Get().Exec([this](void) { this->impl->Realize(realized); });
		}
	}

	void Mesh::DropRealized() {
		Check<BadRealizeStateException>(realized, "Cannot drop the realized representation of an unrealized mesh; it does not exist!");

		realized = false;
		impl->DropRealized();
	}
}