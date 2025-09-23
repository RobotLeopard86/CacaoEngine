#include "UI/UIRenderable.hpp"
#include "UI/Text.hpp"
#include "UI/Image.hpp"

#include <vector>
#include <sstream>
#include <string>

#include "hb-ft.h"
#include "hb-icu.h"
#include "hb.h"
#include "unicode/ucnv.h"
#include "unicode/unistr.h"

namespace Cacao {
	void UIElement::CommonRenderableSetup(std::shared_ptr<UIRenderable> out, glm::uvec2 screenSize) {
		//Copy depth value
		out->depth = depth;

		//Calculate pixel size
		out->size = {round(screenSize.x * size.x), round(screenSize.y * size.y)};

		//Calculate pixel position
		switch(anchor) {
			case AnchorPoint::Center:
				out->screenPos = {round(screenSize.x / 2), round(screenSize.y / 2)};
				break;
			case AnchorPoint::TopLeft:
				out->screenPos = {round(out->size.x / 2), round(out->size.y / 2)};
				break;
			case AnchorPoint::TopRight:
				out->screenPos = {screenSize.x - round(out->size.x / 2), round(out->size.y / 2)};
				break;
			case AnchorPoint::BottomLeft:
				out->screenPos = {round(out->size.x / 2), screenSize.y - round(out->size.y / 2)};
				break;
			case AnchorPoint::BottomRight:
				out->screenPos = {screenSize.x - round(out->size.x / 2), screenSize.y - round(out->size.y / 2)};
				break;
			case AnchorPoint::TopCenter:
				out->screenPos = {round(screenSize.x / 2), round(out->size.y / 2)};
				break;
			case AnchorPoint::BottomCenter:
				out->screenPos = {round(screenSize.x / 2), screenSize.y - round(out->size.y / 2)};
				break;
			case AnchorPoint::LeftCenter:
				out->screenPos = {round(out->size.x / 2), round(screenSize.y / 2)};
				break;
			case AnchorPoint::RightCenter:
				out->screenPos = {screenSize.x - round(out->size.x / 2), round(screenSize.y / 2)};
				break;
		}
		out->screenPos.x += round(offsetFromAnchor.x * screenSize.x);
		out->screenPos.y += round(offsetFromAnchor.y * screenSize.y);
	}

	std::shared_ptr<UIRenderable> Text::MakeRenderable(glm::uvec2 screenSize) {
		//Initial renderable setup
		std::shared_ptr<Renderable> ret = std::make_shared<Renderable>();
		CommonRenderableSetup(std::static_pointer_cast<UIRenderable>(ret), screenSize);
		ret->fontFace = font->face;
		ret->alignment = align;

		//Split text string by newlines and do tab characters
		std::vector<std::string> baseLines;
		int tabCounter = 8;
		std::stringstream ss;
		for(char c : text) {
			switch(c) {
				case '\n':
					baseLines.push_back(ss.str());
					ss.str("");
					tabCounter = 8;
					break;
				case '\t':
					for(; tabCounter > 0; tabCounter--) {
						ss << ' ';
					}
					tabCounter = 8;
					break;
				default:
					ss << c;
					break;
			}
		}
		baseLines.push_back(ss.str());

		//Calculate line gap
		double ascender = font->face->ascender / 64.0;
		double descender = font->face->descender / 64.0;
		double height = font->face->height / 64.0;
		ret->linegap = (height - (ascender - descender)) / height;

		//Calculate font size and line height
		ret->charSize = round((double(ret->size.y) / baseLines.size()) * (1.0 - ret->linegap));
		ret->lineHeight = round(ret->charSize * (ret->linegap + 1));

		//Create Harfbuzz font representation
		FT_Set_Pixel_Sizes(font->face, 0, ret->charSize);
		ret->hbf = hb_ft_font_create(font->face, nullptr);

		//Convert to glyph info
		for(std::string line : baseLines) {
			Renderable::Line rline;

			//Create Harfbuzz buffer
			rline.buffer = hb_buffer_create();
			hb_unicode_funcs_t* icufunctions = hb_icu_get_unicode_funcs();
			hb_buffer_set_unicode_funcs(rline.buffer, icufunctions);

			//Shape line text
			hb_buffer_clear_contents(rline.buffer);
			hb_buffer_add_utf8(rline.buffer, line.c_str(), line.length(), 0, line.length());
			hb_buffer_guess_segment_properties(rline.buffer);
			hb_shape(ret->hbf, rline.buffer, nullptr, 0);

			//Convert to glyphs
			rline.glyphInfo = hb_buffer_get_glyph_infos(rline.buffer, &(rline.glyphCount));
			auto gp = hb_buffer_get_glyph_positions(rline.buffer, &(rline.glyphCount));
			for(unsigned int i = 0; i < rline.glyphCount; i++) {
				rline.advances.emplace_back(gp[i].x_advance, gp[i].y_advance, gp[i].x_offset, gp[i].y_offset);
			}

			//Add to list
			ret->lines.push_back(rline);
		}

		//Calculate color, adjusted for shader
		ret->color = {float(color.r > 0 ? color.r + 1 : 0) / 256.0f,
			float(color.g > 0 ? color.g + 1 : 0) / 256.0f,
			float(color.b > 0 ? color.b + 1 : 0) / 256.0f};

		return std::static_pointer_cast<UIRenderable>(ret);
	}

	std::shared_ptr<UIRenderable> Image::MakeRenderable(glm::uvec2 screenSize) {
		//Initial renderable setup
		std::shared_ptr<Renderable> ret = std::make_shared<Renderable>();
		CommonRenderableSetup(std::static_pointer_cast<UIRenderable>(ret), screenSize);

		//Copy texture handle
		ret->tex = img;

		return std::static_pointer_cast<UIRenderable>(ret);
	}
}