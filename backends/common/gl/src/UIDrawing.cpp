#include "glad/gl.h"

#include "UI/Text.hpp"
#include "UI/Image.hpp"
#include "Core/Exception.hpp"
#include "UI/Shaders.hpp"
#include "GLUtils.hpp"

//Special value that helps align single-line text to the anchor point properly (it looks too high otherwise)
#define SINGLE_LINE_ALIGNMENT (float(screenSize.y) * (linegap / 2))

namespace Cacao {
	struct VBOEntry {
		glm::vec2 vert;
		glm::vec2 tc;
	};

	void Text::Renderable::Draw(glm::uvec2 screenSize, const glm::mat4& projection) {
		//Configure OpenGL (ES)
		GLint originalUnpack;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &originalUnpack);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//Create temporary OpenGL (ES) objects
		GLuint vao, vbo, tex;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenTextures(1, &tex);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VBOEntry) * 6, nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VBOEntry), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VBOEntry), (void*)offsetof(VBOEntry, tc));
		glBindVertexArray(vao);
		glBindVertexArray(0);

		//Write lines
		int lineCounter = 0;
		for(Line ln : lines) {
			//Calculate starting position
			float startX = -((signed int)size.x / 2);
			float startY = (lines.size() > 1 ? (lineHeight * lineCounter) : SINGLE_LINE_ALIGNMENT);
			if(alignment != TextAlign::Left) {
				float textWidth = 0.0f;
				for(unsigned int i = 0; i < ln.glyphCount; ++i) {
					textWidth += (ln.advances[i].adv.x / 64.0f);
				}
				if(alignment == TextAlign::Center) {
					startX = (float(size.x) - textWidth) / 2.0;
				} else {
					startX = float(size.x) - textWidth - (size.x / 2u);
				}
			}
			startX += screenPos.x;
			startY += screenPos.y;
			startY = (screenSize.y - startY);

			//Draw glyphs
			float x = startX, y = startY;
			glBindVertexArray(vao);
			FT_Set_Pixel_Sizes(fontFace, 0, charSize);
			for(unsigned int i = 0; i < ln.glyphCount; i++) {
				FT_Error err = FT_Load_Glyph(fontFace, ln.glyphInfo[i].codepoint, FT_LOAD_RENDER);
				CheckException(!err, Exception::GetExceptionCodeFromMeaning("External"), "Failed to load glyph from font!")

				//Get bitmap
				FT_Bitmap& bitmap = fontFace->glyph->bitmap;

				//Calculate VBO for glyph
				float w = bitmap.width;
				float h = bitmap.rows;
				float xpos = x + fontFace->glyph->bitmap_left;
				float ypos = y - (h - fontFace->glyph->bitmap_top);
				VBOEntry vboData[6] = {
					{{xpos, ypos + h}, {0.0f, 0.0f}},
					{{xpos + w, ypos}, {1.0f, 1.0f}},
					{{xpos, ypos}, {0.0f, 1.0f}},
					{{xpos, ypos + h}, {0.0f, 0.0f}},
					{{xpos + w, ypos + h}, {1.0f, 0.0f}},
					{{xpos + w, ypos}, {1.0f, 1.0f}}};

				//Update VBO
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vboData), vboData);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				//Upload glyph texture to GPU
				glBindTexture(GL_TEXTURE_2D, tex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap.width, bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				//Upload uniforms
				TextShaders::shader->Bind();
				ShaderUploadData up;
				RawGLTexture upTex = {.texObj = tex, .slot = new int(-1)};
				up.emplace_back(ShaderUploadItem {.target = "glyph", .data = std::any(upTex)});
				up.emplace_back(ShaderUploadItem {.target = "color", .data = std::any(color)});
				TextShaders::shader->UploadData(up);

				//Draw glyph
				glDrawArrays(GL_TRIANGLES, 0, 6);

				//Unbind objects
				glActiveTexture(GL_TEXTURE0 + (*upTex.slot));
				glBindTexture(GL_TEXTURE_2D, 0);
				TextShaders::shader->Unbind();

				//Advance cursor for next glyph
				x += (ln.advances[i].adv.x / 64.0f);
				y += (ln.advances[i].adv.y / 64.0f);
			}
			glBindVertexArray(0);

			//Destroy Harfbuzz buffer
			hb_buffer_destroy(ln.buffer);

			//Increment counter
			lineCounter++;
		}
		glBindVertexArray(0);

		//Reset OpenGL (ES)
		glPixelStorei(GL_UNPACK_ALIGNMENT, originalUnpack);

		//Destroy Harfbuzz font representation
		hb_font_destroy(hbf);

		//Clean up OpenGL (ES) objects
		glDeleteTextures(1, &tex);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	void Image::Renderable::Draw(glm::uvec2 screenSize, const glm::mat4& projection) {
		//Get position of top left
		glm::uvec2 topLeft = screenPos - (size / 2u);
		topLeft.y = screenSize.y - topLeft.y;

		//Create vertex buffer
		VBOEntry vboData[6] = {
			{{topLeft.x, topLeft.y - size.y}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y}, {1.0f, 1.0f}},
			{{topLeft.x, topLeft.y}, {0.0f, 1.0f}},
			{{topLeft.x, topLeft.y - size.y}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y - size.y}, {1.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y}, {1.0f, 1.0f}}};

		//Create temporary OpenGL (ES) objects
		GLuint vao, vbo;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VBOEntry) * 6, vboData, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VBOEntry), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VBOEntry), (void*)offsetof(VBOEntry, tc));
		glBindVertexArray(vao);

		//Upload uniforms
		ImageShaders::shader->Bind();
		ShaderUploadData up;
		up.emplace_back(ShaderUploadItem {.target = "image", .data = std::any(tex.GetManagedAsset().get())});
		ImageShaders::shader->UploadData(up);

		//Draw image
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Unbind objects
		tex->Unbind();
		ImageShaders::shader->Unbind();
		glBindVertexArray(0);

		//Clean up OpenGL (ES) objects
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
}