#include "UI/UIRenderable.hpp"
#include "UI/Text.hpp"

#include <vector>
#include <sstream>
#include <string>

#include "hb-ft.h"
#include "hb-icu.h"
#include "hb.h"
#include "unicode/ucnv.h"
#include "unicode/unistr.h"

namespace Cacao {
	std::shared_ptr<UIRenderable> Text::MakeRenderable(glm::uvec2 screenSize) {
		//Initial renderable setup
		std::shared_ptr<Renderable> ret = std::make_shared<Renderable>();
		ret->fontFace = font->face;
		ret->alignment = align;
		ret->depth = depth;

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

		//Create Harfbuzz font representation
		ret->hbf = hb_ft_font_create(font->face, NULL);

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
			hb_shape(ret->hbf, rline.buffer, NULL, 0);

			//Convert to glyphs
			rline.glyphInfo = hb_buffer_get_glyph_infos(rline.buffer, &(rline.glyphCount));
			rline.glyphPositions = hb_buffer_get_glyph_positions(rline.buffer, &(rline.glyphCount));

			//Add to list
			ret->lines.push_back(rline);
		}

		//Calculate pixel size and rotation
		ret->size = {round(screenSize.x * size.x), round(screenSize.y * size.y)};
		ret->rot = rotation;

		//Calculate position
		switch(anchor) {
			case AnchorPoint::Center:
				ret->screenPos = {round(screenSize.x / 2), round(screenSize.y / 2)};
				break;
			case AnchorPoint::TopLeft:
				ret->screenPos = {round(ret->size.x / 2), round(ret->size.y / 2)};
				break;
			case AnchorPoint::TopRight:
				ret->screenPos = {screenSize.x - round(ret->size.x / 2), round(ret->size.y / 2)};
				break;
			case AnchorPoint::BottomLeft:
				ret->screenPos = {round(ret->size.x / 2), screenSize.y - round(ret->size.y / 2)};
				break;
			case AnchorPoint::BottomRight:
				ret->screenPos = {screenSize.x - round(ret->size.x / 2), screenSize.y - round(ret->size.y / 2)};
				break;
			case AnchorPoint::TopCenter:
				ret->screenPos = {round(screenSize.x / 2), round(ret->size.y / 2)};
				break;
			case AnchorPoint::BottomCenter:
				ret->screenPos = {round(screenSize.x / 2), screenSize.y - round(ret->size.y / 2)};
				break;
			case AnchorPoint::LeftCenter:
				ret->screenPos = {round(ret->size.x / 2), round(screenSize.y / 2)};
				break;
			case AnchorPoint::RightCenter:
				ret->screenPos = {screenSize.x - round(ret->size.x / 2), round(screenSize.y / 2)};
				break;
		}
		ret->screenPos.x += round(offsetFromAnchor.x * screenSize.x);
		ret->screenPos.y += round(offsetFromAnchor.y * screenSize.y);

		//Calculate color, adjusted for shader
		ret->color = {float(color.r > 0 ? color.r + 1 : 0) / 256.0f,
			float(color.g > 0 ? color.g + 1 : 0) / 256.0f,
			float(color.b > 0 ? color.b + 1 : 0) / 256.0f};

		//Calculate line gap
		double ascender = font->face->ascender / 64.0;
		double descender = font->face->descender / 64.0;
		double height = font->face->height / 64.0;
		ret->linegap = (height - (ascender - descender)) / height;

		//Calculate font size and line height
		ret->charSize = round((double(ret->size.y) / ret->lines.size()) * (1.0 - ret->linegap));
		ret->lineHeight = round(ret->charSize * (ret->linegap + 1));

		return std::static_pointer_cast<UIRenderable>(ret);
	}
}