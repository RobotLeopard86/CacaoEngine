#include "UI/Shaders.hpp"

#include "GLHeaders.hpp"

namespace Cacao {
	void GenShaders() {
		//Text element
		ShaderSpec tspec;
		tspec.emplace_back(ShaderItemInfo {.type = SpvType::SampledImage, .size = {1, 1}, .entryName = "glyph"});
		tspec.emplace_back(ShaderItemInfo {.type = SpvType::Float, .size = {3, 1}, .entryName = "color"});
		std::vector<uint32_t> v(TextShaders::vertex, std::end(TextShaders::vertex));
		std::vector<uint32_t> f(TextShaders::fragment, std::end(TextShaders::fragment));
		TextShaders::shader = new Shader(v, f, tspec);
		TextShaders::shader->Compile();
	}

	void DelShaders() {
		TextShaders::shader->Release();
		delete TextShaders::shader;
	}
}