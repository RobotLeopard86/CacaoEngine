#pragma once

#include "Shader.hpp"

namespace Cacao {
	//Material which holds a shader and data for the shader
	class Material {
	  public:
		//Current shader
		AssetHandle<Shader> shader;

		//Data storage
		ShaderUploadData data;
	};
}