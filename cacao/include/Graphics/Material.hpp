#pragma once

#include "Shader.hpp"

namespace Cacao {
	///@brief A shader and data to input into it
	struct Material {
	  public:
		AssetHandle<Shader> shader;///<The shader to use
		ShaderUploadData data;	   ///<The data to upload
	};
}