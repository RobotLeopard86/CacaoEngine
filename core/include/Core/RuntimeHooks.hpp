#pragma once

#include "Graphics/Shader.hpp"

#include <string>

namespace Cacao {
	void RTStartup();
	void RTShutdown();
	std::shared_ptr<Shader> RTLoadShader(std::string assetID);
	std::shared_ptr<Mesh> RTLoadMesh(std::string assetID);
	std::shared_ptr<Sound> RTLoadSound(std::string assetID);
	std::shared_ptr<Font> RTLoadFont(std::string assetID);
	std::shared_ptr<Texture2D> RTLoadTex2D(std::string assetID);
	std::shared_ptr<Cubemap> RTLoadCubemap(std::string assetID);
}