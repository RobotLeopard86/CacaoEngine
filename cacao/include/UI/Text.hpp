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
		glm::vec3 GetColor() {
			return color;
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
		void SetColor(glm::vec3 c) {
			color = c;
			dirty = true;
		}

		struct Renderable : public UIRenderable {
			struct Line {
				hb_glyph_info_t* glyphInfo;
				hb_glyph_position_t* glyphPositions;
				hb_buffer_t* buffer;
				unsigned int glyphCount;
			};
			std::vector<Line> lines;
			FT_Face fontFace;
			TextAlign alignment;
			glm::vec3 color;
			double linegap;
			unsigned int lineHeight;
			FT_F26Dot6 charSize;
			hb_font_t* hbf;

			void Draw(glm::uvec2 screenSize, const glm::mat4& projection) override;
		};

		std::shared_ptr<UIRenderable> MakeRenderable(glm::uvec2 screenSize) override;

	  protected:
		//Text to display
		std::string text;

		//Font face
		AssetHandle<Font> font;

		//Text color
		//0-255 for red, green, blue
		glm::u8vec3 color;

		//Alignment
		TextAlign align;
	};
}