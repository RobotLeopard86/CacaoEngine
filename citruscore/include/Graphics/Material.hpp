#pragma once

#include "Shader.hpp"

namespace Citrus {
	//Shader data base struct
	struct ShaderData {};
	
	//Material which holds a shader and data for the shader
	class Material {
	public:
		//Current shader
		Shader* shader;

		//Access the stored shader data to modify it
		ShaderData& operator->(){
			return shaderData;
		}
	private:
		//Data storage
		ShaderData shaderData;
	};
}