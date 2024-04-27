#pragma once

#include "Texture.hpp"
#include "Utilities/MiscUtils.hpp"

#include "glm/vec2.hpp"

#include <string>

namespace Cacao {

	//2D texture
	//Must be implemented per-rendering API
	class Texture2D : public Texture {
	  public:
		Texture2D(std::string filePath);
		~Texture2D() {
			if(bound) Unbind();
			if(compiled) Release();

			delete dataBuffer;
			delete nativeData;
		}

		//Attach this texture to the specified slot
		void Bind(int slot) override;
		//Detach this texture
		void Unbind() override;
		//Compile texture to be used later
		std::shared_future<void> Compile() override;
		//Delete texture when no longer needed
		void Release() override;

		std::string GetType() override {
			return "2DTEX";
		}

	  protected:
		unsigned char* dataBuffer;
		glm::ivec2 imgSize;
		int numImgChannels;

		NativeData* nativeData;
	};
}