#pragma once

#include "UIElement.hpp"
#include "Font.hpp"
#include "Utilities/Asset.hpp"
#include "UIRenderable.hpp"

namespace Cacao {
	//Horizontal alignment of a text within a text box
	enum class TextAlign {
		Left,
		Center,
		Right
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
		TextAlign GetAlignment() {
			return align;
		}

		void SetText(std::string t) {
			text = t;
			dirty = true;
		}
		void SetFont(AssetHandle<Font> f) {
			font = f;
			dirty = true;
		}
		void SetAlignment(TextAlign a) {
			align = a;
			dirty = true;
		}

		struct Renderable : public UIRenderable {
			struct Line {
				hb_glyph_info_t* glyphInfo;
				hb_glyph_position_t* glyphPositions;
			};
			std::vector<Line> lines;
			FT_Face fontFace;
			TextAlign alignment;
		};

		UIRenderable MakeRenderable(glm::uvec2 screenSize) override;

	  protected:
		//Text to display
		std::string text;

		//Font face
		AssetHandle<Font> font;

		//Alignment
		TextAlign align;
	};
}