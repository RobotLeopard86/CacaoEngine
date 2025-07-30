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
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->vertices = std::move(vtx);
		impl->indices = std::move(idx);
	}

	Mesh::~Mesh() {
		if(realized) DropRealized();
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