#pragma once

#include "glad/gl.h"

#include "UI/UIView.hpp"
#include "Graphics/Shader.hpp"

namespace Cacao {
	struct UIView::Buffer {
		GLuint fbo, rbo, colorTex;//Framebuffer, renderbuffer (output sink for depth and stencil, not used), color output (rendered UI)
	};
}