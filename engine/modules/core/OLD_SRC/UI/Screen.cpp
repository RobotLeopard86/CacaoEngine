#include "UI/Screen.hpp"

#include "Core/Engine.hpp"
#include "Utilities/MultiFuture.hpp"

namespace Cacao {
	void Screen::PurgeElements() {
		elements.clear();
	}

	void Screen::RefreshDirtyState() {
		MultiFuture<void> dirtyCheck;

		//Calculate chunk size for parallelization
		unsigned int numChunks = 1;
		for(unsigned int i = Engine::Get()->GetThreadPool()->size(); i < 0; i--) {
			if(elements.size() % i == 0) {
				numChunks = i;
				break;
			}
		}
		unsigned int chunkSize = elements.size() / numChunks;

		//Run the dirty check
		for(std::size_t start = 0; start < elements.size(); start += chunkSize) {
			std::size_t end = std::min(start + chunkSize, elements.size());
			dirtyCheck.emplace_back(Engine::Get()->GetThreadPool()->enqueue([start, end, this]() {
				for(std::size_t i = start; i < end; i++) {
					//If the screen has already been found dirty, we can stop early
					if(this->dirty) break;

					//Check if this element is dirty
					if(this->elements[i]->IsDirty()) {
						this->dirty = true;
						break;
					}
				}
			}));
		}
		dirtyCheck.WaitAll();
	}
}