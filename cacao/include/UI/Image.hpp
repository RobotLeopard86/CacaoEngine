#pragma once

#include "UIElement.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Utilities/Asset.hpp"
#include "UIRenderable.hpp"

namespace Cacao {
	//An image element
	class Image final : public UIElement {
	  public:
		AssetHandle<Texture2D> GetImage() {
			return img;
		}
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