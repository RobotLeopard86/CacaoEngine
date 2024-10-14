#include "UI/Shaders.hpp"

namespace Cacao {
	void GenShaders() {
		//Text element
		ShaderSpec tspec;
		tspec.emplace_back(ShaderItemInfo {.type = SpvType::SampledImage, .size = {1, 1}, .entryName = "glyph"});
		tspec.emplace_back(ShaderItemInfo {.type = SpvType::Float, .size = {3, 1}, .entryName = "color"});
		std::vector<uint32_t> tV(TextShaders::vertex, std::end(TextShaders::vertex));
		std::vector<uint32_t> tF(TextShaders::fragment, std::end(TextShaders::fragment));
		TextShaders::shader = new Shader(tV, tF, tspec);
		TextShaders::shader->Compile();

		//Image element
		ShaderSpec ispec;
		ispec.emplace_back(ShaderItemInfo {.type = SpvType::SampledImage, .size = {1, 1}, .entryName = "image"});
		std::vector<uint32_t> iV(ImageShaders::vertex, std::end(ImageShaders::vertex));
		std::vector<uint32_t> iF(ImageShaders::fragment, std::end(ImageShaders::fragment));
		ImageShaders::shader = new Shader(iV, iF, ispec);
		ImageShaders::shader->Compile();
	}

	void DelShaders() {
		TextShaders::shader->Release();
		ImageShaders::shader->Release();
		delete ImageShaders::shader;
	}
}