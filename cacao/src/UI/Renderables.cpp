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
	UIRenderable Text::MakeRenderable(glm::uvec2 screenSize) {
		//Initial renderable setup
		Renderable ret;
		ret.fontFace = font->face;
		ret.alignment = align;
		ret.depth = depth;

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

		//Create Harfbuzz buffer
		hb_buffer_t* buf = hb_buffer_create();

		//Configure Harfbuzz Unicode
		hb_unicode_funcs_t* icufunctions = hb_icu_get_unicode_funcs();
		hb_buffer_set_unicode_funcs(buf, icufunctions);

		//Convert to glyph info
		for(std::string line : baseLines) {
			//Shape line text
			hb_buffer_clear_contents(buf);
			hb_buffer_add_utf8(buf, line.c_str(), line.length(), 0, line.length());
			hb_buffer_guess_segment_properties(buf);
			hb_shape(font->hbFont, buf, NULL, 0);

			//Convert to glyphs
			unsigned int glyphCount;
			Renderable::Line rline;
			rline.glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
			rline.glyphPositions = hb_buffer_get_glyph_positions(buf, &glyphCount);

			//Add to list
			ret.lines.push_back(rline);
		}

		//Calculate pixel size and rotation
		ret.size = {round(screenSize.x * size.x), round(screenSize.y * size.y)};
		ret.rot = rotation;

		//Calculate position
		switch(anchor) {
			case AnchorPoint::Center:
				ret.screenPos = {round(screenSize.x / 2), round(screenSize.y / 2)};
				break;
			case AnchorPoint::TopLeft:
				ret.screenPos = {round(ret.size.x / 2), round(ret.size.y / 2)};
				break;
			case AnchorPoint::TopRight:
				ret.screenPos = {screenSize.x - round(ret.size.x / 2), round(ret.size.y / 2)};
				break;
			case AnchorPoint::BottomLeft:
				ret.screenPos = {round(ret.size.x / 2), screenSize.y - round(ret.size.y / 2)};
				break;
			case AnchorPoint::BottomRight:
				ret.screenPos = {screenSize.x - round(ret.size.x / 2), screenSize.y - round(ret.size.y / 2)};
				break;
			case AnchorPoint::TopCenter:
				ret.screenPos = {round(screenSize.x / 2), round(ret.size.y / 2)};
				break;
			case AnchorPoint::BottomCenter:
				ret.screenPos = {round(screenSize.x / 2), screenSize.y - round(ret.size.y / 2)};
				break;
			case AnchorPoint::LeftCenter:
				ret.screenPos = {round(screenSize.x / 2), round(ret.size.y / 2)};
				break;
			case AnchorPoint::RightCenter:
				ret.screenPos = {screenSize.x - round(screenSize.x / 2), round(ret.size.y / 2)};
				break;
		}
		ret.screenPos.x += round(screenSize.x * size.x);
		ret.screenPos.y += round(screenSize.y * size.y);

		return ret;
	}
}