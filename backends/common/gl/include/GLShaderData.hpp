#pragma once

#include "Graphics/Shader.hpp"

#include "glad/gl.h"

#include <string>
#include <map>

namespace Cacao {
	//Struct for data required for an OpenGL shader
	struct ShaderDataImpl {
		GLuint gpuID;							//ID of the shader program
		std::string vertexCode, fragmentCode;	//Processed GLSL source
		bool unusedTransform;					//Whether the shader doesn't use the transform value (and thus may have it optimized out)
		GLuint transformLoc;					//Transform uniform location
		std::map<std::string, uint32_t> offsets;//Named offsets into the shader data (how to arrange the shader data)
		GLuint shaderUBOBinding;				//Binding number of the shader's object data uniform buffer
		bool hasImages;							//If the shader has images in its spec
		uint32_t shaderDataSize;				//Total size of shader data

		static GLuint uboIndexCounter;
	};

	inline ShaderDataImpl* activeShader;

	//Wrapper for the shader class
	struct Shader::ShaderData {
		ShaderDataImpl impl;
	};
}