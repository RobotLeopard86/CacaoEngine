#pragma once

#include "Screen.hpp"
#include "Graphics/Shader.hpp"

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

		//Has this UI view been rendered?
		bool HasBeenRendered() {
			return hasRendered;
		}

		UIView();
		~UIView();

	  private:
		glm::uvec2 size;

		std::shared_ptr<Screen> screen;

		bool bound;
		int currentSlot;

		//Has the view been rendered at least once?
		//If not, we can't bind
		bool hasRendered;

		//Backend-implemented view buffer
		struct Buffer;
		std::shared_ptr<Buffer> frontBuffer;
		std::shared_ptr<Buffer> backBuffer;

		//Vertex shader SPIR-V code (and yes, the name joke is intentional)
		//Generated by glslc at build-time
		static constexpr uint32_t vsCode[] =
#include "uiquad.vert.txt"
			;

		//Fragment shader SPIR-V code
		//Generated by glslc at build-time
		static constexpr uint32_t fsCode[] =
#include "uiquad.frag.txt"
			;

		//Shader object
		static Shader* shader;

		friend class UIViewShaderManager;

		//Draw the processed renderables to the front buffer
		//Backend-implemented
		void Draw(std::map<unsigned short, std::vector<std::shared_ptr<UIRenderable>>> renderables);
	};
}