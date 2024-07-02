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
		void Bind(int slot) {
			frontBuffer.Bind(slot);
		}

		//Unbind the front buffer
		void Unbind() {
			frontBuffer.Unbind();
		}

		//Set the currently displayed screen
		//No change will be seen until Render() is called
		void SetScreen(std::weak_ptr<Screen> s) {
			screen = s;
		}

	  private:
		glm::uvec2 size;
		Texture2D frontBuffer;
		Texture2D backBuffer;

		std::weak_ptr<Screen> screen;
	};
}