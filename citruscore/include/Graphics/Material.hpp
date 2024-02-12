#pragma once

#include "Shader.hpp"

namespace Citrus {
	//Material which holds a shader and data for the shader
	class Material {
	public:
		//Current shader
		Shader* shader;

		//Data storage
		ShaderData shaderData;
	};
}