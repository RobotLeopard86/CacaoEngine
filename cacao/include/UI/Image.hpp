#pragma once

#include "UIElement.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Utilities/Asset.hpp"
#include "UIRenderable.hpp"

namespace Cacao {
	//An image element
	class Image final : public UIElement {
	  public:
		/**
		 * @brief Get current image to display
		 *
		 * @return The current image
		 */
		AssetHandle<Texture2D> GetImage() {
			return img;
		}

		/**
		 * @brief Set the image to display and make this element dirty
		 *
		 * @param i The new image
		 */
		void SetImage(AssetHandle<Texture2D> i) {
			img = i;
			dirty = true;
		}

		struct Renderable : public UIRenderable {
			AssetHandle<Texture2D> tex;

			void Draw(glm::uvec2 screenSize, const glm::mat4& projection) override;
		};

		std::shared_ptr<UIRenderable> MakeRenderable(glm::uvec2 screenSize) override;

	  private:
		//The image to draw
		AssetHandle<Texture2D> img;
	};
}