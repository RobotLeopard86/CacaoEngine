#pragma once

#include "Graphics/Shader.hpp"
#include "UI/UIView.hpp"

namespace Cacao {
	class UIViewShaderManager {
	  public:
		void Compile() {
			//Define UI view shader specification
			ShaderSpec spec;
			ShaderItemInfo samplerInfo;
			samplerInfo.entryName = "uiTex";
			samplerInfo.size = {1, 1};
			samplerInfo.type = SpvType::SampledImage;
			spec.push_back(samplerInfo);

			//Create temporary data objects
			std::vector<uint32_t> v(UIView::vsCode, std::end(UIView::vsCode));
			std::vector<uint32_t> f(UIView::fsCode, std::end(UIView::fsCode));

			//Create and compile UI view shader object
			//Compile future is intentionally discarded as it will be done by the time this is used
			UIView::shader = new Shader(v, f, spec);
			UIView::shader->Compile();
		}
		void Release() {
			UIView::shader->Release();
			delete UIView::shader;
		}
		Shader* operator->() {
			return UIView::shader;
		}
	};

	inline UIViewShaderManager uivsm;
}