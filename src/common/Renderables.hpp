#pragma once

#include "UI/Text.hpp"
#include "UI/Image.hpp"

#include "glm/glm.hpp"
#include "hb.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#include <vector>

namespace Cacao {
	struct Text::Renderable : public UIRenderable {
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

	struct Image::Renderable : public UIRenderable {
		AssetHandle<Texture2D> tex;

		void Draw(glm::uvec2 screenSize, const glm::mat4& projection) override;
	};
}