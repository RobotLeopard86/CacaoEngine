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
		template<class T>
		T& AccessData(){
			static_assert(std::is_base_of<ShaderData, T>(), "Must access this data using a subclass of ShaderData!");
			return shaderData;
		}
	private:
		//Data storage
		ShaderData shaderData;
	};
}