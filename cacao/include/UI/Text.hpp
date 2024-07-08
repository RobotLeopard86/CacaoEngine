#pragma once

#include "UIElement.hpp"
#include "Font.hpp"
#include "Utilities/Asset.hpp"

namespace Cacao {
	//Horizontal alignment of a text within a text box
	enum class TextAlign {
		Left,
		Center,
		Right
	};

	//Vertical alignment of a text within a text box
	enum class VerticalTextAlign {
		Top,
		Center,
		Bottom
	};

	//A text element
	//Font size is automatically calculated based on element size
	class Text final : public UIElement {
	  public:
		std::string GetText() {
			return text;
		}
		AssetHandle<Font> GetFont() {
			return font;
		}
		TextAlign GetHorizontalAlignment() {
			return hAlign;
		}
		VerticalTextAlign GetVerticalAlignment() {
			return vAlign;
		}

		void SetText(std::string t) {
			text = t;
			dirty = true;
		}
		void SetFont(AssetHandle<Font> f) {
			font = f;
			dirty = true;
		}
		void SetHorizontalAlignment(TextAlign ha) {
			hAlign = ha;
			dirty = true;
		}
		void SetVerticalAlignment(VerticalTextAlign va) {
			vAlign = va;
			dirty = true;
		}

	  protected:
		void Draw(glm::uvec2 areaPixelSize) override;

		//Text to display
		std::string text;

		//Font face
		AssetHandle<Font> font;

		//Alignment
		TextAlign hAlign;
		VerticalTextAlign vAlign;
	};
}