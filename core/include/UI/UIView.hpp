#pragma once

#include "Screen.hpp"
#include "Graphics/Shader.hpp"

#include "glm/glm.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Renderer for a Screen
	 */
	class UIView {
	  public:
		/**
		 * @brief Render the selected screen and swap buffers
		 *
		 * @throws Exception If there is no screen to render
		 */
		void Render();

		/**
		 * @brief Attach the front buffer to a texture slot
		 *
		 * @param slot The texture slot to attach to
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the view hasn't been rendered, already bound, or if not called on the engine thread
		 */
		void Bind(int slot);

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the view isn't bound, or if not called on the engine thread
		 */
		void Unbind();

		/**
		 * @brief Set the current screen
		 *
		 * @param s The new screen to render
		 */
		void SetScreen(std::shared_ptr<Screen> s) {
			screen = s;

			//Mark as dirty so the renderer knows to re-render
			s->dirty = true;
		}

		/**
		 * @brief Get the current screen
		 *
		 * @return The current screen
		 */
		std::shared_ptr<Screen> GetScreen() {
			return screen;
		}

		/**
		 * @brief Set the size of the rendered area
		 *
		 * @param sz The new size in pixels
		 */
		void SetSize(glm::uvec2 sz) {
			//Mark the screen dirty to force a re-render at the new size
			if(screen) screen->ForceDirty();
			size = sz;
		}

		/**
		 * @brief Get the size of the rendered area
		 *
		 * @return The current size in pixels
		 */
		glm::uvec2 GetSize() {
			return size;
		}

		/**
		 * @brief Check if this view has been rendered
		 *
		 * @return Whether the view has been rendered or not
		 */
		bool HasBeenRendered() {
			return hasRendered;
		}

		/**
		 * @brief Create a new UI view
		 *
		 * @throws Exception If the framebuffers could not be created
		 *
		 * @warning If the rendering backend has not been initialized yet, this function will block until it is
		 */
		UIView();

		/**
		 * @brief Destroy the UI view
		 */
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