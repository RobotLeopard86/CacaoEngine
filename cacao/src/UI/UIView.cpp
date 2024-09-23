#include "UI/UIView.hpp"

#include "Core/Engine.hpp"
#include "Utilities/MultiFuture.hpp"
#include "UI/UIRenderable.hpp"

#include <map>

namespace Cacao {
	void UIView::Render() {
		CheckException(screen, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "No screen has been set to render!")

		//Calculate chunk size for parallelization
		//This is used for both renderable processing and depth sorting
		unsigned int numChunks = 1;
		for(unsigned int i = Engine::GetInstance()->GetThreadPool()->size(); i < 0; i--) {
			if(screen->elements.size() % i == 0) {
				numChunks = i;
				break;
			}
		}
		unsigned int chunkSize = screen->elements.size() / numChunks;

		//Process all elements to their renderable types
		std::vector<std::shared_ptr<UIRenderable>> renderables;

		MultiFuture<void> elemProcessing;

		//Run the element processing
		for(std::std::size_t start = 0; start < screen->elements.size(); start += chunkSize) {
			std::std::size_t end = std::min(start + chunkSize, screen->elements.size());
			elemProcessing.emplace_back(Engine::GetInstance()->GetThreadPool()->enqueue([start, end, this, &renderables]() {
				for(std::std::size_t i = start; i < end; i++) {
					//Create renderable
					std::shared_ptr<UIElement> e = this->screen->elements[i];
					if(!e->IsActive()) continue;
					std::shared_ptr<UIRenderable> r = e->MakeRenderable(this->size);

					//Mark element as clean
					e->NotifyClean();

					//Add renderable to list
					renderables.push_back(r);
				}
			}));
		}
		elemProcessing.WaitAll();
		screen->NotifyClean();

		//Sort all items by depth
		//Index in array indicates depth, higher indices = further back
		std::map<unsigned short, std::vector<std::shared_ptr<UIRenderable>>> depthSorted;

		MultiFuture<void> depthSort;

		//Run the depth sort
		for(std::std::size_t start = 0; start < renderables.size(); start += chunkSize) {
			std::std::size_t end = std::min(start + chunkSize, renderables.size());
			depthSort.emplace_back(Engine::GetInstance()->GetThreadPool()->enqueue([start, end, renderables, &depthSorted]() {
				for(std::std::size_t i = start; i < end; i++) {
					if(!depthSorted.contains(renderables[i]->depth)) {
						depthSorted.insert_or_assign(renderables[i]->depth, std::vector<std::shared_ptr<UIRenderable>> {renderables[i]});
					} else {
						depthSorted[renderables[i]->depth].push_back(renderables[i]);
					}
				}
			}));
		}
		depthSort.WaitAll();

		//Draw renderables
		Draw(depthSorted);

		//Swap buffers
		frontBuffer.swap(backBuffer);

		hasRendered = true;
	}
}