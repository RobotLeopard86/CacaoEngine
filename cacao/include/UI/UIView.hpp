#pragma once

#include "Graphics/Textures/Texture2D.hpp"
#include "Screen.hpp"

#include "glm/glm.hpp"

#include <memory>

namespace Cacao {
	//A view for UI
	class UIView {
	  public:
		//Render a snapshot of the current screen and swap buffers
		void Render();

		//Bind the front buffer to a texture slot
		void Bind(int slot);

		//Unbind the front buffer
		void Unbind();

		//Set the currently displayed screen
		//No change will be seen until Render() is called
		void SetScreen(std::shared_ptr<Screen> s) {
			screen = s;

			//Mark as dirty so the renderer knows to re-render
			s->dirty = true;
		}

		//Get the currently displayed screen
		std::shared_ptr<Screen> GetScreen() {
			return screen;
		}

		//Set the size (in pixels) of the rendered area
		void SetSize(glm::uvec2 sz) {
			size = sz;
		}

		//Get the size (in pixels) of the rendered area
		glm::uvec2 GetSize() {
			return size;
		}

	  private:
		glm::uvec2 size;

		std::shared_ptr<Screen> screen;

		//Backend-implemented view buffer
		struct Buffer;

		std::shared_ptr<Buffer> frontBuffer;
		std::shared_ptr<Buffer> backBuffer;

		//Draw the processed renderables to the front buffer
		//Backend-implemented
		void Draw(std::map<unsigned short, std::vector<UIRenderable>> renderables);
	};
}