#include "GLHeaders.hpp"

#include "UI/Text.hpp"
#include "Core/Exception.hpp"
#include "UI/Shaders.hpp"
#include "GLUtils.hpp"

namespace Cacao {
	void Text::Renderable::Draw(const glm::mat4& projection) {
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 7, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindVertexArray(vao);
		glBindVertexArray(0);

		//Write lines
		int lineCounter = 0;
		for(Line ln : lines) {
			//Calculate starting position
			float startX = 0;
			float startY = lineHeight * lineCounter;
			if(alignment != TextAlign::Left) {
				float textWidth = 0.0f;
				for(unsigned int i = 0; i < ln.glyphCount; ++i) {
					textWidth += (ln.glyphPositions[i].x_advance / 64.0f);
				}
				if(alignment == TextAlign::Center) {
					startX = (float(size.x) - textWidth) / 2.0;
				} else {
					startX = float(size.x) - textWidth;
				}
			}
			startX += screenPos.x;
			startY += screenPos.y;

			//Draw glyphs
			float x = startX, y = startY;
			glBindVertexArray(vao);
			for(unsigned int i = 0; i < ln.glyphCount; i++) {
				CheckException(FT_Load_Glyph(fontFace, ln.glyphInfo[i].codepoint, FT_LOAD_RENDER), Exception::GetExceptionCodeFromMeaning("External"), "Failed to load glyph from font!")

				//Get bitmap
				FT_Bitmap& bitmap = fontFace->glyph->bitmap;

				//Calculate VBO for glyph
				float w = bitmap.width;
				float h = bitmap.rows;
				float xpos = x + fontFace->glyph->bitmap_left;
				float ypos = y - (h - fontFace->glyph->bitmap_top);
				GLfloat vboData[6][7] = {
					{xpos, ypos + h, 0.0f, 0.0f, color.r, color.g, color.b},
					{xpos + w, ypos, 1.0f, 1.0f, color.r, color.g, color.b},
					{xpos, ypos, 0.0f, 1.0f, color.r, color.g, color.b},
					{xpos, ypos + h, 0.0f, 0.0f, color.r, color.g, color.b},
					{xpos + w, ypos + h, 1.0f, 0.0f, color.r, color.g, color.b},
					{xpos + w, ypos, 1.0f, 1.0f, color.r, color.g, color.b}};

				//Update VBO
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vboData), vboData);

				//Upload glyph texture to GPU
				glBindTexture(GL_TEXTURE_2D, tex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap.width, bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				//Upload uniforms
				TextShaders::shader->Bind();
				TextShaders::shader->UploadCacaoData(projection, glm::identity<glm::mat4>(), glm::identity<glm::mat4>());
				ShaderUploadData up;
				RawGLTexture upTex = {.texObj = tex, .slot = NULL};
				up.emplace_back(ShaderUploadItem {.target = "glyph", .data = std::any(upTex)});
				TextShaders::shader->UploadData(up);

				//Draw glyph
				glDrawArrays(GL_TRIANGLES, 0, 6);

				//Unbind objects
				glActiveTexture(GL_TEXTURE0 + (*upTex.slot));
				glBindTexture(GL_TEXTURE_2D, 0);
				TextShaders::shader->Unbind();

				//Advance cursor for next glyph
				x += (ln.glyphPositions[i].x_advance / 64.0f);
				y += (ln.glyphPositions[i].y_advance / 64.0f);
			}
			glBindVertexArray(0);

			//Increment counter
			lineCounter++;
		}

		//Reset OpenGL (ES)
		glPixelStorei(GL_UNPACK_ALIGNMENT, originalUnpack);
	}
}