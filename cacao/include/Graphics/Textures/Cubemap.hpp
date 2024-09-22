#pragma once

#include "Texture.hpp"
#include "Utilities/MiscUtils.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace Cacao {

	//Cubemap texture
	//Must be implemented per-rendering API
	class Cubemap final : public Texture {
	  public:
		//Order of faces: +X, -X, +Y, -Y, +Z, -Z
		Cubemap(std::vector<std::string> filePaths);

		~Cubemap() final {
			if(bound) Unbind();
			if(compiled) Release();
		}

		//Attach this cubemap to the specified slot
		void Bind(int slot) override;
		//Detach this cubemap
		void Unbind() override;
		//Compile cubemap to be used later
		std::shared_future<void> Compile() override;
		//Delete cubemap when no longer needed
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() override {
			return "CUBEMAP";
		}

	  protected:
		//Backend-implemented data type
		struct CubemapData;

		std::vector<std::string> textures;

		std::shared_ptr<CubemapData> nativeData;
	};
}