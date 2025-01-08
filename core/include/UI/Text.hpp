#pragma once

#include "UIElement.hpp"
#include "Font.hpp"
#include "Utilities/Asset.hpp"
#include "UIRenderable.hpp"

namespace Cacao {
	///@brief The alignment of text within a text box
	enum class TextAlign {
		Left,
		Center,
		Right
	};

	/**
	 * @brief A text UI element
	 */
	class Text final : public UIElement {
	  public:
		/**
		 * @brief Get the current text
		 *
		 * @return The text
		 */
		std::string GetText() {
			return text;
		}

		/**
		 * @brief Get the current font face
		 *
		 * @return The font face
		 */
		AssetHandle<Font> GetFont() {
			return font;
		}

		/**
		 * @brief Get the current horizontal alignment
		 *
		 * @return The alignment
		 */
		TextAlign GetAlignment() {
			return align;
		}

		/**
		 * @brief Get the current text color
		 *
		 * @return The text color
		 */
		glm::vec3 GetColor() {
			return color;
		}

		/**
		 * @brief Set the text and make this element dirty
		 *
		 * @param t The new text
		 */
		void SetText(std::string t) {
			text = t;
			dirty = true;
		}

		/**
		 * @brief Set the font face and make this element dirty
		 *
		 * @param f The new font face
		 */
		void SetFont(AssetHandle<Font> f) {
			font = f;
			dirty = true;
		}

		/**
		 * @brief Set the horizontal alignment and make this element dirty
		 *
		 * @param a The new alignment
		 */
		void SetAlignment(TextAlign a) {
			align = a;
			dirty = true;
		}

		/**
		 * @brief Set the text color and make this element dirty
		 *
		 * @param c The new color
		 */
		void SetColor(glm::vec3 c) {
			color = c;
			dirty = true;
		}

		struct Renderable : public UIRenderable {
			struct Advance {
				glm::i32vec2 adv, offset;
				Advance(int32_t xa, int32_t ya, int32_t xo, int32_t yo)
				  : adv(xa, ya), offset(xo, yo) {}
			};
			struct Line {
				hb_glyph_info_t* glyphInfo;
				std::vector<Advance> advances;
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